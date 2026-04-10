#include "file_based_layer_setting_dialog.h"
#include "ui_file_based_layer_setting_dialog.h"
#include <editor/settings/file_based_layer_setting.h>
#include <gui/utility/gui_utility.h>
#include <QFileDialog>
#include <QFileInfo>

namespace alive {
FileBasedLayerSettingDialog::FileBasedLayerSettingDialog(model::FileBasedLayerSetting &setting,
                                                         QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui::FileBasedLayerSettingDialog)
    , m_setting(setting)
{
    m_ui->setupUi(this);
    m_ui->name->setText(QString::fromStdString(setting.name));
    m_ui->filePath->setText(QString::fromStdString(setting.path));

    connect(m_ui->name, &QLineEdit::textChanged, this, [this](const QString &text) {
        if (!alive::gui::Utility::instance()->validate_object_name(text)) {
            m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        } else {
            m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
            m_setting.name = text.toStdString();
            emit setting_changed();
        }
    });

    connect(m_ui->filePath, &QPushButton::clicked, this, [this]() {
        auto path = QFileDialog::getOpenFileName(this,
                                                 tr("Select File"),
                                                 QFileInfo(QString::fromStdString(m_setting.path)).dir().path());
        if (!path.isEmpty()) {
            m_setting.path = path.toStdString();
            m_ui->filePath->setText(path);
            emit setting_changed();
        }
    });
}

FileBasedLayerSettingDialog::~FileBasedLayerSettingDialog()
{
    delete m_ui;
}
} // namespace alive
