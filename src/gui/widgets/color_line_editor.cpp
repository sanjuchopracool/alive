#include "color_line_editor.h"
#include "color_selection_label.h"
#include "utility.h"
#include <gui/color_utility.h>
#include <QColorDialog>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

namespace alive {
ColorLineEditor::ColorLineEditor(QWidget *parent)
    : QWidget{parent}
{
    QHBoxLayout *layout = hbox_layout();
    m_label = new ColorSelectionLabel(this);
    m_edit = new QLineEdit(this);
    layout->addWidget(m_label);
    layout->addWidget(m_edit);
    // layout->addStretch(1);
    setLayout(layout);
    show_alpha(false);

    connect(m_label, &ColorSelectionLabel::color_changed, this, [this](const QColor &color) {
        m_edit->setText(color.name());
        emit color_changed(color);
    });

    connect(m_label, &ColorSelectionLabel::clicked, this, [this]() {
        QColor prev_color = m_label->color();
        QColorDialog dialog(prev_color);
        if (m_show_alpha) {
            dialog.setOption(QColorDialog::ShowAlphaChannel);
        }

        auto set_color_from_dialog = [this](const QColor &color) {
            set_color(color);
            emit color_changed(color);
        };

        QObject::connect(&dialog, &QColorDialog::currentColorChanged, this, set_color_from_dialog);
        int result = dialog.exec();
        if (result == QDialog::Rejected) {
            set_color_from_dialog(prev_color);
        }
    });

    connect(m_edit, &QLineEdit::textEdited, this, [this](const QString &color) {
        if (m_edit->hasAcceptableInput()) {
            if (color.size() == 7) {
                m_label->set_color(gui::from_rgb_string(color));
            } else {
                m_label->set_color(gui::from_rgba_string(color));
            }
            emit color_changed(m_label->color());
        }
    });
}

void ColorLineEditor::set_color(const QColor &color)
{
    m_label->set_color(color);
    m_edit->setText(m_show_alpha ? gui::to_rgba_string(color) : color.name());
}

const QColor &ColorLineEditor::color() const
{
    return m_label->color();
}

void ColorLineEditor::show_alpha(bool show)
{
    m_show_alpha = show;

    auto rx = show ? QRegularExpression("^#(?:[0-9a-fA-F]{4}){1,2}$")
                   : QRegularExpression("^#(?:[0-9a-fA-F]{3}){1,2}$");
    QValidator *validator = new QRegularExpressionValidator(rx, this);
    m_edit->setValidator(validator);
}

} // namespace alive
