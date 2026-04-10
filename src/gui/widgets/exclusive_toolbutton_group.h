#ifndef EXCLUSIVE_TOOLBUTTON_GROUP_H
#define EXCLUSIVE_TOOLBUTTON_GROUP_H

#include <QSet>
#include <QToolButton>

namespace alive {

class ExclusiveToolButtonGroup : public QWidget
{
    Q_OBJECT
public:
    explicit ExclusiveToolButtonGroup(Qt::Orientation orientation, QWidget *parent = nullptr);

    void add_tool_button(QToolButton *button);

    void select(QToolButton *button, bool checked, bool emit_signal = false);
signals:
    void tool_button_checked(QToolButton *btn);

private:
    QLayout *m_layout = nullptr;
    QSet<QToolButton *> m_buttons;
};
} // namespace alive

#endif // EXCLUSIVE_TOOLBUTTON_GROUP_H
