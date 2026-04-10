#include "main_window.h"
#include "menubar.h"
#include <core/importer/importer.h>
#include <gui/dialogs/composition_setting_dialog.h>
#include <gui/dialogs/file_based_layer_setting_dialog.h>
#include <gui/dialogs/solid_layer_setting_dialog.h>
#include <gui/document/composition_document.h>
#include <editor/settings/file_based_layer_setting.h>
#include <editor/settings/solid_layer_setting.h>
#include <gui/project/project_manager.h>
#include <gui/settings/user_ui_setting.h>
#include <gui/widgets/central_widget.h>
#include <gui/widgets/utility.h>
#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <core/constants/file_extension.h>

namespace alive {

struct MainWindowPrivateData
{
    CentralWidget *main_widget = nullptr;
    MenuBar *menu_bar = nullptr;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_d(std::make_unique<MainWindowPrivateData>())
{
    setMenuBar(m_d->menu_bar = new MenuBar(this));
    m_d->main_widget = new CentralWidget(this);
    setCentralWidget(m_d->main_widget);
    restore_settings();
    auto *prj_mgr = alive::project::ProjectManager::instance();
    connect(prj_mgr,
            &alive::project::ProjectManager::window_title_changed,
            this,
            [=, this](const std::string &title) {
                this->setWindowTitle(QString::fromStdString(title));
            });

    connect(prj_mgr,
            &alive::project::ProjectManager::close_unmodified_doc,
            this,
            &MainWindow::slot_close_modified_doc);

    connect(prj_mgr,
            &alive::project::ProjectManager::show_composition_setting_dialog,
            this,
            &MainWindow::slot_show_comp_setting_dialog);
    connect(prj_mgr,
            &alive::project::ProjectManager::show_setting_for_solid_layer,
            this,
            &MainWindow::slot_show_solid_setting_dialog);
    connect(prj_mgr,
            &alive::project::ProjectManager::show_setting_for_file_based_layer,
            this,
            &MainWindow::slot_show_setting_for_file_based_layer_dialog);
    connect(prj_mgr,
            &alive::project::ProjectManager::save_new_comp,
            this,
            &MainWindow::slot_show_save_comp_dialog);
}

MainWindow::~MainWindow()
{
    save_settings();
}

CanvasTabWidget *MainWindow::canvas_tab_widget()
{
    return m_d->main_widget->canvas_tab_widget();
}

int MainWindow::slot_close_modified_doc(alive::model::CompositionDocument* doc)
{
    auto *prj_mgr = alive::project::ProjectManager::instance();
    QString doc_name = QString::fromStdString(doc->saved_path());
    if (doc_name.isEmpty()) {
        doc_name = doc->name();
    }
    QMessageBox warning_box(QMessageBox::Warning,
                            tr("Save Modifed Document"),
                            tr("%1 has been modified. Do you want to save it?").arg(doc_name),
                            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                            this);
    // warning_box.setIconPixmap(QIcon::fromTheme("eye").pixmap(100, 100));
    int response = warning_box.exec();
    if (response == QMessageBox::Yes) {
        prj_mgr->save_and_close_doc_with_id(doc->id());
    } else if (response == QMessageBox::No) {
        prj_mgr->slot_close_document(doc);
    }

    return response;
}

int MainWindow::slot_show_comp_setting_dialog(CompositionSetting setting)
{
    CompositionSettingDialog setting_dlg(setting, this);
    int response = setting_dlg.exec();
    if (response == QDialog::Accepted) {
        auto *prj_mgr = alive::project::ProjectManager::instance();
        if (setting.document) {
            prj_mgr->update_composition_setting(setting);
        } else {
            prj_mgr->create_new_composition_from_setting(setting);
        }
    }
    return response;
}

int MainWindow::slot_show_solid_setting_dialog(alive::model::Layer *layer, const model::LayerSetting &data)
{
    if (!layer) {
        return -1;
    }

    alive::model::SolidLayerSetting old_setting = std::get<alive::model::SolidLayerSetting>(data);
    alive::model::SolidLayerSetting new_setting = std::get<alive::model::SolidLayerSetting>(data);
    SolidLayerSettingDialog setting_dlg(new_setting, this);
    setting_dlg.setWindowTitle(QString::fromStdString(layer->name()) + tr(" Layer Setting"));
    auto *prj_mgr = alive::project::ProjectManager::instance();
    connect(&setting_dlg, &SolidLayerSettingDialog::setting_changed, this, [&]() {
        if (layer) {
            prj_mgr->apply_layer_settings(layer, new_setting);
        }
    });

    int response = setting_dlg.exec();
    if (response == QDialog::Rejected && old_setting != new_setting) {
        prj_mgr->apply_layer_settings(layer, old_setting);
    }
    return response;
}

int MainWindow::slot_show_setting_for_file_based_layer_dialog(model::Layer *layer,
                                                              const model::LayerSetting &data)
{
    alive::model::FileBasedLayerSetting old_setting
        = std::get<alive::model::FileBasedLayerSetting>(data);
    alive::model::FileBasedLayerSetting new_setting
        = std::get<alive::model::FileBasedLayerSetting>(data);
    FileBasedLayerSettingDialog setting_dlg(new_setting, this);
    setting_dlg.setWindowTitle(QString::fromStdString(layer->name()) + tr(" Layer Setting"));
    auto *prj_mgr = alive::project::ProjectManager::instance();
    connect(&setting_dlg, &FileBasedLayerSettingDialog::setting_changed, this, [&]() {
        if (layer) {
            prj_mgr->apply_layer_settings(layer, new_setting);
        }
    });

    int response = setting_dlg.exec();
    if (response == QDialog::Rejected && old_setting != new_setting) {
        prj_mgr->apply_layer_settings(layer, old_setting);
    }
    return response;
}

void MainWindow::slot_show_save_comp_dialog(const QString &name)
{
    auto *prj_mgr = alive::project::ProjectManager::instance();
    auto *setting = UserSettings::instance();
    QString selected_filter;
    const QString filter_alive ("Alive Files (*.alive)");
    QString file_path = QFileDialog::getSaveFileName(this,
                                                tr("Save Composition"),
                                                setting->last_opened_file_dir_url().toLocalFile(),
                                                filter_alive,
                                                &selected_filter);
    if (!file_path.isEmpty()) {
        auto ext = "." + QString(file_extensions::k_alive);
        if (!file_path.endsWith(ext)) {
            file_path = file_path + ext;
        }

        prj_mgr->save_as(file_path);
    }
}

void MainWindow::slot_export_as(alive::exporter::Exporter::Format format)
{
    auto *prj_mgr = alive::project::ProjectManager::instance();
    auto *setting = UserSettings::instance();
    if (format == alive::exporter::Exporter::e_PNGSequence) {
        QString dir = QFileDialog::getExistingDirectory(
            this,
            tr("Open directory to export %1 as Image Sequence").arg(prj_mgr->current_doc_name()),
            setting->last_opened_file_dir_url().toLocalFile(),
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if (!dir.isEmpty()) {
            prj_mgr->export_as_image_sequence(QUrl::fromLocalFile(dir));
        }
    } else {
        QUrl file_url
            = QFileDialog::getSaveFileUrl(this,
                                          tr("Export %1 as WebP").arg(prj_mgr->current_doc_name()),
                                          setting->last_opened_file_dir_url(),
                                          tr("WebP File (*.webp)"));
        if (file_url.isValid() && file_url.isLocalFile()) {
            prj_mgr->export_as_webp(file_url);
        }
    }
}

void MainWindow::slot_import()
{
    // TODO check for persistance
    auto *prj_mgr = alive::project::ProjectManager::instance();
    auto *setting = UserSettings::instance();
    QUrl file_url = QFileDialog::getOpenFileUrl(this,
                                                tr("Import File"),
                                                setting->last_opened_file_dir_url(),
                                                tr("Svg Files (*.svg)"));
    if (file_url.isValid() && file_url.isLocalFile()) {
        Messages messages;
        auto layers = importer::import_file(std::filesystem::path(
                                                file_url.toLocalFile().toStdString()),
                                            messages);

        bool load = true;
        // TODO Display messages
        for (const auto &message : messages) {
            qDebug() << message.message;
            if (message.type == MessageType::e_Error) {
                load = false;
            }
        }

        if (load && layers.size()) {
            prj_mgr->import(layers);
        }
    }
}

void MainWindow::update_recent_files_menu(ActionContextItem *item)
{
    m_d->menu_bar->update_recent_menu(item);
}

void MainWindow::closeEvent(QCloseEvent *ev)
{
    auto *prj_mgr = alive::project::ProjectManager::instance();
    QList<alive::model::CompositionDocument *> modifed_docs = prj_mgr->close_unmodifed_docs();
    if (modifed_docs.size()) {
        for (alive::model::CompositionDocument *doc : modifed_docs) {
            if (slot_close_modified_doc(doc) == QMessageBox::Cancel) {
                ev->ignore();
                return;
            }
        }
    }

    QMainWindow::closeEvent(ev);
}

void MainWindow::restore_settings()
{
    QVariant geometry = alive::UserSettings::instance()->main_window_geometry();
    if (geometry.isValid()) {
        restoreGeometry(geometry.toByteArray());
    } else {
        showMaximized();
    }
    m_d->main_widget->restore_settings();
}

void MainWindow::save_settings()
{
    m_d->main_widget->save_settings();
    alive::UserSettings::instance()->save_main_window_geometry(saveGeometry());
}
} // namespace alive
