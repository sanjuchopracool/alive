#include "checkable_icon.h"
#include <QIcon>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>

namespace alive {
CheckableIcon::CheckableIcon(const QSize &size, const QString &name, QWidget *parent, bool checked)
    : QWidget{parent}
    , m_checked(true)
{
    m_icon = QIcon::fromTheme(name);
    setFixedSize(size);
}

void CheckableIcon::set_checked(bool clicked)
{
    m_checked = clicked;
    update();
}

void CheckableIcon::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    if (m_checked) {
        QPainter p(this);
        m_icon.paint(&p, rect());
    }
}

void CheckableIcon::mousePressEvent(QMouseEvent *event)
{
    set_checked(!m_checked);
    emit checked(m_checked);
    event->accept();
}
} // namespace alive
