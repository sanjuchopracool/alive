#include "solid_layer_setting_dialog.h"
#include "ui_solid_layer_setting_dialog.h"
#include <editor/settings/solid_layer_setting.h>
#include <gui/utility/gui_utility.h>
#include <QKeyEvent>
#include <QPushButton>
#include <gui/color_utility.h>

namespace alive {
SolidLayerSettingDialog::SolidLayerSettingDialog(alive::model::SolidLayerSetting &setting,
                                                 QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui::SolidLayerSetting)
    , m_setting(setting)
{
    m_ui->setupUi(this);
    m_ui->size_editor->set_suffix(tr("px"));

    m_ui->name->setText(QString::fromStdString(setting.name));
    m_ui->size_editor->set_range(1, std::numeric_limits<int>::max());
    m_ui->size_editor->set_value({setting.width, setting.height});
    m_ui->color_editor->set_color(gui::from_sk_color(setting.color));
    m_ui->size_editor->set_linked(true);

    setFixedSize(sizeHint().width() * 1.5, sizeHint().height());
    setWindowTitle(tr("Solid Setting"));

    connect(m_ui->name, &QLineEdit::textChanged, this, [this](const QString &text) {
        if (!alive::gui::Utility::instance()->validate_object_name(text)) {
            m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        } else {
            m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
            m_setting.name = text.toStdString();
            emit setting_changed();
        }
    });

    connect(m_ui->size_editor, &LinkedIntEditor::value_changed, this, [this](const IntPair &val) {
        m_setting.width = val.first;
        m_setting.height = val.second;
        emit setting_changed();
    });

    connect(m_ui->color_editor, &ColorLineEditor::color_changed, this, [this](const QColor &val) {
        m_setting.color = gui::to_color4ub(val);
        emit setting_changed();
    });
}

void SolidLayerSettingDialog::keyPressEvent(QKeyEvent *evt)
{
    if (evt->key() == Qt::Key_Enter || evt->key() == Qt::Key_Return)
        return;
    QDialog::keyPressEvent(evt);
}

SolidLayerSettingDialog::~SolidLayerSettingDialog()
{
    delete m_ui;
}
} // namespace alive
