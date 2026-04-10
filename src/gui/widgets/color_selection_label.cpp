#include "color_selection_label.h"
#include <QMouseEvent>
#include <QPainter>

namespace alive {
ColorSelectionLabel::ColorSelectionLabel(QWidget *parent)
    : QWidget{parent}
{}

void ColorSelectionLabel::set_color(const QColor &color)
{
    if (m_color != color) {
        m_color = color;
        update();
    }
}

QSize ColorSelectionLabel::sizeHint() const
{
    return QSize(30, 20);
}

void ColorSelectionLabel::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.fillRect(this->rect(), Qt::white);
    p.fillRect(this->rect(), m_color);
}

void ColorSelectionLabel::mousePressEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton) {
        emit clicked();
        ev->accept();
        return;
    }

    QWidget::mousePressEvent(ev);
}
} // namespace alive
