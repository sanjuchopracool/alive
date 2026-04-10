#include "application.h"
#include "action_ids.h"
#include <core/logging/logger.h>
#include <gui/action_contexts.h>
#include <gui/framework/managers/action_manager.h>
#include <gui/framework/theme/alive_style.h>
#include <gui/gui_initializer.h>
#include <gui/managers/dock_action_manager.h>
#include <gui/project/project_manager.h>
#include <gui/settings/user_settings.h>
#include <gui/settings/user_ui_setting.h>
#include <gui/widgets/canvas_tab_widget.h>
#include <gui/widgets/dock_widget.h>
#include <gui/widgets/main_window.h>
#include <tuple>
#include <QDir>
#include <QFileDialog>
#include <QFontDatabase>
#include <QIcon>
#include <QStringList>
#include <QStyleFactory>

namespace alive {

const char k_app_context[] = "APPLICATION";
const char k_panels_context[] = "PANELS";;

void core_message_handler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    using namespace alive;
    QByteArray local_message = msg.toLocal8Bit();
    const char *file = context.file ? context.file : "";
    const char *function = context.function ? context.function : "";
    switch (type) {
    case QtDebugMsg:
        ALIVE_CORE_DEBUG("{}", local_message.constData());
        break;
    case QtInfoMsg:
        ALIVE_CORE_INFO("{}", local_message.constData());
        break;
    case QtWarningMsg:
        ALIVE_CORE_WARN("{} ({}:{}, {})", local_message.constData(), file, context.line, function);
        break;
    case QtFatalMsg:
    case QtCriticalMsg:
        ALIVE_CORE_CRITICAL("{} ({}:{}, {})", local_message.constData(), file, context.line, function);
        break;
    }
}

struct ApplicationPrivateData
{
    ApplicationPrivateData()
    {
        auto resource_path = QDir::home();
        resource_path.cd("alive_resources");
        std::string abs_path = resource_path.absolutePath().toStdString();
        alive::core::Initializer::Params param{spdlog::level::level_enum::debug, abs_path};
        initializer.reset(std::make_unique<alive::Initializer>(param).release());
    }
    std::unique_ptr<alive::Initializer> initializer;
    DockWidget *focused_dock = nullptr;
    MainWindow *main_window = nullptr;
    ActionContextItem *recent_menu_context = nullptr;
};

Application::Application(int &argc, char **argv, int flags)
    : QApplication(argc, argv, flags)
    , m_d{std::make_unique<ApplicationPrivateData>()}
{
    init();
    connect(this, &QApplication::focusChanged, this, &Application::handle_focus_change);
}

Application::~Application() {}

int Application::execute()
{
    MainWindow w;
    w.show();

    m_d->main_window = &w;

    QStyle *style = QStyleFactory::create("Fusion");
    if (!style) {
        alive::ALIVE_CORE_ERROR("Could not find Fusion Style!");
    }

    auto *alive_style = new alive::theme::Style(style);
    setStyle(alive_style);
    {
        alive::project::ProjectManager::instance()->open_file(
            alive::settings::UserSettings::instance()->last_opened_file_url());
    }
    return exec();
}

void Application::init()
{
    qInstallMessageHandler(core_message_handler);

    // FONTS
    QFontDatabase::addApplicationFont(":/fonts/Roboto/Roboto-Black.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Roboto/Roboto-BlackItalic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Roboto/Roboto-Bold.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Roboto/Roboto-BoldItalic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Roboto/Roboto-Italic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Roboto/Roboto-Light.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Roboto/Roboto-LightItalic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Roboto/Roboto-Medium.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Roboto/Roboto-MediumItalic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Roboto/Roboto-Regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Roboto/Roboto-Thin.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Roboto/Roboto-ThinItalic.ttf");

    qreal point_size = font().pointSizeF();
    QFont font("Roboto");
    font.setPointSizeF(point_size);
    setFont(font);

    QString path = QDir::homePath() + "/PROJECTS/alive/icons";
    QIcon::setThemeSearchPaths(QStringList() << path);
    alive::ALIVE_CORE_DEBUG("Theme search paths: {}",
                           QIcon::themeSearchPaths().join(',').toStdString());
    QIcon::setThemeName("dark");

    create_actions();
}

void Application::create_actions()
{
    using namespace action_ids;
    using namespace action_context_ids;
    QAction *action = nullptr;
    auto app_context = ActionManager::instance()->create_root_context(k_app_context,
                                                                      tr("Application"),
                                                                      ActionManager::e_App);
    alive::project::ProjectManager *prj_mgr = alive::project::ProjectManager::instance();

    // File
    auto sub_context = app_context->add_context("APP.FILE", tr("&File"));
    action = new QAction("&New", this);
    // action->setShortcut(
    //     QKeySequence(QKeyCombination(Qt::CTRL, Qt::Key_F), QKeyCombination(Qt::CTRL, Qt::Key_N)));
    action->setShortcut(QKeySequence::New);
    connect(action, &QAction::triggered, this, [=]() { prj_mgr->new_composition(); });
    sub_context->add_action(k_file_new_action, action);

    action = new QAction("&Open", this);
    action->setShortcut(QKeySequence::Open);
    sub_context->add_action(k_file_open_action, action);
    connect(action, &QAction::triggered, this, [=]() {
        auto *setting = UserSettings::instance();
        QUrl file_url = QFileDialog::getOpenFileUrl(nullptr,
                                                    tr("Open Composition"),
                                                    setting->last_opened_file_dir_url(),
                                                    tr("Alive Files (*.alive *.json)"));
        if (file_url.isValid() && file_url.isLocalFile()) {
            prj_mgr->open_file(file_url);
            add_to_recent_files(file_url.toLocalFile());
        }
    });

    {
        m_d->recent_menu_context = sub_context->add_context(k_recent_files_key,
                                                            tr("&Recent Compositions"));
        update_recent_files_actions();
    }

    action = new QAction("&Close", this);
    action->setShortcut(QKeySequence::Close);
    sub_context->add_action(k_file_close_action, action);
    connect(action, &QAction::triggered, this, [=]() { prj_mgr->close(); });

    action = new QAction("&Save", this);
    action->setShortcut(QKeySequence::Save);
    sub_context->add_action(k_file_save_action, action);
    connect(action, &QAction::triggered, this, [=]() { prj_mgr->save(); });

    action = new QAction("&Save As..", this);
    action->setShortcut(QKeySequence::SaveAs);
    sub_context->add_action(k_file_save_as_action, action);
    connect(action, &QAction::triggered, this, [=]() { prj_mgr->trigger_save_as(); });

    QList<QKeySequence> shortcuts;

    action = new QAction("&Import", this);
    sub_context->add_action(k_file_import_action, action);

    shortcuts.clear();
    shortcuts.emplaceBack(QKeyCombination(Qt::Modifier::CTRL, Qt::Key_I));
    action->setShortcuts(shortcuts);
    connect(action, &QAction::triggered, this, [=]() { m_d->main_window->slot_import(); });

    {
        auto *sub_menu = sub_context->add_context("FILE.EXPORT", tr("&Export As"));

        action = new QAction("&Image Sequence", this);
        // action->setShortcut(Qt::Key_Comma);
        sub_menu->add_action(k_file_export_as_png_action, action);
        connect(action, &QAction::triggered, this, [=]() {
            m_d->main_window->slot_export_as(alive::exporter::Exporter::e_PNGSequence);
        });

        auto get_canvas = [this]() { return m_d->main_window->canvas_tab_widget(); };
        action = new QAction("&WebP ", this);
        // action->setShortcut(Qt::Key_Period);
        sub_menu->add_action(k_file_export_as_webp_action, action);
        connect(action, &QAction::triggered, this, [=]() {
            m_d->main_window->slot_export_as(alive::exporter::Exporter::e_WebM);
        });
    }

    // Edit
    sub_context = app_context->add_context("APP.EDIT", tr("&Edit"));
    action = new QAction("&Undo", this);
    action->setShortcut(QKeySequence::Undo);
    sub_context->add_action(k_edit_undo_action, action);
    connect(action, &QAction::triggered, this, [=]() { DockActionManager::instance()->undo(); });

    action = new QAction("&Redo", this);
    action->setShortcut(QKeySequence::Redo);
    sub_context->add_action(k_edit_redo_action, action);
    connect(action, &QAction::triggered, this, [=]() { DockActionManager::instance()->redo(); });

    action = new QAction("&Composition Setting", this);
    sub_context->add_action(k_edit_comp_setting_action, action);
    connect(action, &QAction::triggered, this, [=]() { prj_mgr->edit_composition_setting(); });

    // LAYER
    sub_context = app_context->add_context("APP.LAYER", tr("&Layer"));
    {
        auto *sub_menu = sub_context->add_context("APP.LAYER.NEW", tr("&New Layer"));
        using namespace alive::model;
        std::vector<std::tuple<QString, LayerType, const char *>>
            actions{{tr("&Solid"), LayerType::e_Solid, "LAYER.NEW.SOLID"},
                    {tr("S&hape Layer"), LayerType::e_Shape, "LAYER.NEW.SHAPE"},
                    {tr("&Text Layer"), LayerType::e_Text, "LAYER.NEW.TEXT"},
                    {tr("&Image"), LayerType::e_Image, "LAYER.NEW.IMAGE"},
                    {tr("&Null"), LayerType::e_Null, "LAYER.NEW.NULL"}};
        for (const auto &item : actions) {
            action = new QAction(std::get<0>(item), this);
            connect(action, &QAction::triggered, this, [=]() {
                prj_mgr->create_new_layer(std::get<1>(item));
            });
            sub_menu->add_action(std::get<2>(item), action);
        }
    }

    // view
    sub_context = app_context->add_context("APP.VIEW", tr("&View"));
    {
        auto *sub_menu = sub_context->add_context("APP.VIEW.ZOOM", tr("&Zoom"));

        auto get_canvas = [this]() { return m_d->main_window->canvas_tab_widget(); };
        action = new QAction("Zoom &In", this);
        action->setShortcut(Qt::Key_Period);
        sub_menu->add_action(k_view_zoom_in_action, action);
        connect(action, &QAction::triggered, this, [=]() { get_canvas()->zoom_in(); });

        action = new QAction("Zoom &Out", this);
        action->setShortcut(Qt::Key_Comma);
        sub_menu->add_action(k_view_zoom_out_action, action);
        connect(action, &QAction::triggered, this, [=]() { get_canvas()->zoom_out(); });

        action = new QAction("Zoom &Fit", this);
        action->setShortcut(Qt::Key_5);
        sub_menu->add_action(k_view_zoom_fit_action, action);
        connect(action, &QAction::triggered, this, [=]() { get_canvas()->zoom_fit(); });
    }

    // create panels
    auto panel_context = ActionManager::instance()->create_root_context(k_panels_context,
                                                                        tr("Panels"),
                                                                        ActionManager::e_Panels);
    sub_context = panel_context->add_context(k_panel_timeline_context, tr("Timeline"));

    // timeline
    action = new QAction("Play/Pause", this);
    action->setShortcut(QKeySequence(Qt::Key_Space));
    sub_context->add_action(k_timeline_play_pause_action, action);
    connect(action, &QAction::triggered, this, [=]() {
        DockActionManager::instance()->play_pause();
    });

    {
        auto move_timeline_by = [prj_mgr](int frame) {
            auto timeline = prj_mgr->current_timline();
            if (timeline) {
                timeline->select_frame(timeline->current_frame() + frame);
            }
        };
        shortcuts.clear();
        action = new QAction(tr("Go Forward 1 Frame"), this);
        shortcuts.emplaceBack(QKeySequence(Qt::Key_PageDown));
        shortcuts.emplaceBack(QKeyCombination(Qt::Modifier::CTRL, Qt::Key_Right));
        action->setShortcuts(shortcuts);
        sub_context->add_action(k_timeline_go_next_1_frame_action, action);
        connect(action, &QAction::triggered, this, [=]() { move_timeline_by(1); });

        shortcuts.clear();
        action = new QAction(tr("Go Forward 10 Frame"), this);
        shortcuts.emplaceBack(QKeyCombination(Qt::Modifier::CTRL, Qt::Key_PageDown));
        shortcuts.emplaceBack(
            QKeyCombination(Qt::Modifier::CTRL | Qt::Modifier::SHIFT, Qt::Key_Right));
        action->setShortcuts(shortcuts);
        sub_context->add_action(k_timeline_go_next_10_frame_action, action);
        connect(action, &QAction::triggered, this, [=]() { move_timeline_by(10); });

        shortcuts.clear();
        action = new QAction(tr("Go Back 1 Frame"), this);
        shortcuts.emplaceBack(QKeySequence(Qt::Key_PageUp));
        shortcuts.emplaceBack(QKeyCombination(Qt::Modifier::CTRL, Qt::Key_Left));
        action->setShortcuts(shortcuts);
        sub_context->add_action(k_timeline_go_back_1_frame_action, action);
        connect(action, &QAction::triggered, this, [=]() { move_timeline_by(-1); });

        shortcuts.clear();
        action = new QAction(tr("Go Back 10 Frame"), this);
        shortcuts.emplaceBack(QKeyCombination(Qt::Modifier::CTRL, Qt::Key_PageUp));
        shortcuts.emplaceBack(
            QKeyCombination(Qt::Modifier::CTRL | Qt::Modifier::SHIFT, Qt::Key_Left));
        action->setShortcuts(shortcuts);
        sub_context->add_action(k_timeline_go_back_10_frame_action, action);
        connect(action, &QAction::triggered, this, [=]() { move_timeline_by(-10); });
    }

    action = new QAction(tr("Set Work Area End"), this);
    action->setShortcut(QKeySequence(Qt::Key_B));
    sub_context->add_action(k_timeline_start_work_area_action, action);
    connect(action, &QAction::triggered, this, [=]() { prj_mgr->set_work_area_start(); });

    action = new QAction(tr("Set Work Area End"), this);
    action->setShortcut(QKeySequence(Qt::Key_N));
    sub_context->add_action(k_timeline_end_work_area_action, action);
    connect(action, &QAction::triggered, this, [=]() { prj_mgr->set_work_area_end(); });
}

void Application::update_recent_files_actions(bool clear)
{
    alive::project::ProjectManager *prj_mgr = alive::project::ProjectManager::instance();

    m_d->recent_menu_context->clear_items();
    if (clear) {
        UserSettings::instance()->set_data(action_ids::k_recent_files_key, {});
    } else {
        auto files
            = UserSettings::instance()->get_data(action_ids::k_recent_files_key).toStringList();
        auto files2 = files;

        for (const auto &file : files2) {
            if (!QFileInfo(file).exists()) {
                files.removeOne(file);
                continue;
            }
        }
        if (!files.isEmpty()) {
            for (const auto &file : files) {
                QAction *action = new QAction(file, this);
                m_d->recent_menu_context->add_action(file.toStdString().c_str(), action);
                connect(action, &QAction::triggered, this, [this, file, prj_mgr]() {
                    prj_mgr->open_file(QUrl::fromLocalFile(file));
                    add_to_recent_files(file);
                });
            }

            QAction *action = new QAction(tr("&Clear"), this);
            m_d->recent_menu_context->add_action(action_ids::k_clear_recent_files_key, action);
            connect(action, &QAction::triggered, this, [this]() {
                update_recent_files_actions(true);
            });
        }

        if (files.size() != files2.size()) {
            UserSettings::instance()->set_data(action_ids::k_recent_files_key, files);
        }
    }
    if (m_d->main_window) {
        m_d->main_window->update_recent_files_menu(m_d->recent_menu_context);
    }
}

void Application::add_to_recent_files(const QString &file_path)
{
    auto files = UserSettings::instance()->get_data(action_ids::k_recent_files_key).toStringList();
    if (files.contains(file_path)) {
        files.removeOne(file_path);
    }

    files.prepend(file_path);
    if (files.size() > 10) {
        files.resize(10);
    }
    UserSettings::instance()->set_data(action_ids::k_recent_files_key, files);
    update_recent_files_actions();
}

void Application::handle_focus_change(QWidget *old, QWidget *now)
{
    DockWidget *w = nullptr;
    if (now) {
        QWidget *parent = now;
        while (parent) {
            w = dynamic_cast<DockWidget *>(parent);
            if (w) {
                break;
            }
            parent = parent->parentWidget();
        }
    }

    bool reset = false;
    if (w) {
        if (w != m_d->focused_dock) {
            reset = true;
        }
    } else {
        reset = true;
    }

    if (reset) {
        if (m_d->focused_dock) {
            m_d->focused_dock->set_focus(false);
        }

        m_d->focused_dock = w;
        if (w) {
            w->set_focus(true);
        }
    }

    DockActionManager::instance()->set_current_dock(w);
}
} // namespace alive
