#include "exclusive_toolbutton_group.h"
#include <QToolButton>
#include <gui/widgets/utility.h>

namespace alive {

ExclusiveToolButtonGroup::ExclusiveToolButtonGroup(
    Qt::Orientation orientation, QWidget *parent)
    : QWidget{parent}
{
    if (orientation == Qt::Horizontal) {
        m_layout = hbox_layout();
    } else {
        m_layout = vbox_layout();
    }

    setLayout(m_layout);
}

void ExclusiveToolButtonGroup::add_tool_button(
    QToolButton *button)
{
    if (button && !m_buttons.contains(button)) {
        m_buttons.insert(button);
        m_layout->addWidget(button);
        connect(button, &QToolButton::clicked, this, [this, button](bool checked) {
            select(button, checked, true);
        });
    }
}

void ExclusiveToolButtonGroup::select(
    QToolButton *button, bool checked, bool emit_signal)
{
    if (checked) {
        button->setChecked(true);
        for (auto *btn : m_buttons) {
            if (btn != button) {
                btn->setChecked(false);
            }
        }

        if (emit_signal) {
            emit this->tool_button_checked(button);
        }
    }
}

} // namespace alive
