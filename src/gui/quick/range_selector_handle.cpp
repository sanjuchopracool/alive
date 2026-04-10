#include "range_selector_handle.h"
#include <QPainter>
#include <QPainterPath>

namespace alive::gui {
RangeSelectorHandle::RangeSelectorHandle(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
    connect(this, &QQuickPaintedItem::widthChanged, this, &RangeSelectorHandle::update_path);
    connect(this, &QQuickPaintedItem::heightChanged, this, &RangeSelectorHandle::update_path);
}

void RangeSelectorHandle::paint(QPainter *painter)
{
    painter->save();
    QPen pen(m_border_color, m_border_width);
    painter->setPen(pen);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setBrush(m_color);
    painter->drawPath(m_path);
    painter->restore();
}

void RangeSelectorHandle::set_border_radius(qreal radius)
{
    if (m_border_radius != radius) {
        m_border_radius = radius;
        update_path();
        emit border_radius_changed();
    }
}

void RangeSelectorHandle::update_path()
{
    QRectF rect = this->boundingRect();
    m_path.clear();
    QRectF arc_rect({0, 0}, QSizeF(2 * m_border_radius, 2 * m_border_radius));
    if (m_is_left) {
        m_path.moveTo(rect.topRight());
        qreal mid_x = rect.left() + m_border_radius;
        m_path.lineTo({mid_x, rect.top()});
        qreal mid_y_bottom = rect.bottom() - m_border_radius;
        arc_rect.moveLeft(rect.left());
        arc_rect.moveTop(rect.top());
        m_path.arcTo(arc_rect, 90, 90);
        m_path.lineTo({rect.left(), mid_y_bottom});
        arc_rect.moveBottom(rect.bottom());
        m_path.arcTo(arc_rect, 180, 90);
        m_path.lineTo({mid_x, rect.bottom()});
        m_path.lineTo(rect.bottomRight());
        m_path.closeSubpath();
    } else {
        m_path.moveTo(rect.topLeft());
        m_path.lineTo(rect.bottomLeft());
        qreal mid_x = rect.right() - m_border_radius;
        m_path.lineTo({mid_x, rect.bottom()});
        arc_rect.moveBottom(rect.bottom());
        arc_rect.moveRight(rect.right());
        m_path.arcTo(arc_rect, 270, 90);
        qreal mid_y_top = rect.top() + m_border_radius;
        m_path.lineTo({rect.right(), mid_y_top});
        arc_rect.moveTop(rect.top());
        m_path.arcTo(arc_rect, 0, 90);
        m_path.closeSubpath();
    }
}
} // namespace alive::gui
