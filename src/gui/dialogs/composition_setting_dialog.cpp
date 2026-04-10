#include "composition_setting_dialog.h"
#include "ui_composition_setting_dialog.h"
#include <editor/settings/composition_setting.h>
#include <gui/utility/gui_utility.h>
#include <gui/color_utility.h>
#include <QKeyEvent>
#include <QPushButton>
namespace alive {
CompositionSettingDialog::CompositionSettingDialog(CompositionSetting &setting, QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui::CompositionSettingDialog)
    , m_setting(setting)
{
    m_ui->setupUi(this);
    m_ui->size_editor->set_suffix(tr("px"));
    m_ui->duration_editor->set_suffix(tr("seconds"));
    constexpr int min = 10;
    constexpr int max = std::numeric_limits<int>::max();
    m_ui->size_editor->set_range(min, max);
    m_ui->frame_rate_editor->set_range(1, max);
    m_ui->duration_editor->set_range(1, max);

    m_ui->lineEdit->setText(QString::fromStdString(setting.name));
    m_ui->size_editor->set_value({setting.width, setting.height});
    m_ui->size_editor->set_linked(true);
    m_ui->frame_rate_editor->set_value(setting.fps);
    m_ui->duration_editor->set_value(setting.duration);
    m_ui->bg_color_selector->set_color(gui::from_sk_color(setting.bg_color));
    m_ui->bg_color_selector->setEnabled(setting.show_bg_color);
    m_ui->bg_checkbox->setCheckState(setting.show_bg_color ? Qt::CheckState::Checked
                                                           : Qt::CheckState::Unchecked);

    setFixedSize(sizeHint().width() * 1.5, sizeHint().height());
    setWindowTitle(tr("Composition Setting"));

    connect(m_ui->duration_editor, &IntEditor::value_changed, this, [this](int value) {
        m_setting.duration = value;
    });

    connect(m_ui->frame_rate_editor, &DoubleEditor::value_changed, this, [this](double value) {
        m_setting.fps = value;
    });

    connect(m_ui->lineEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        if (!alive::gui::Utility::instance()->validate_object_name(text)) {
            m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        } else {
            m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
            m_setting.name = text.toStdString();
        }
    });

    connect(m_ui->size_editor, &LinkedIntEditor::value_changed, this, [this](const IntPair val) {
        m_setting.width = val.first;
        m_setting.height = val.second;
    });

    connect(m_ui->bg_color_selector,
            &ColorLineEditor::color_changed,
            this,
            [this](const QColor &val) { m_setting.bg_color = gui::to_sk_color(val); });

    connect(m_ui->bg_checkbox, &QCheckBox::checkStateChanged, this, [this](Qt::CheckState state) {
        bool show_bg_color = state == Qt::CheckState::Checked;
        m_ui->bg_color_selector->setEnabled(show_bg_color);
        m_setting.show_bg_color = show_bg_color;
    });
}

void CompositionSettingDialog::keyPressEvent(QKeyEvent *evt)
{
    if (evt->key() == Qt::Key_Enter || evt->key() == Qt::Key_Return)
        return;
    QDialog::keyPressEvent(evt);
}

CompositionSettingDialog::~CompositionSettingDialog()
{
    delete m_ui;
}
} // namespace alive
