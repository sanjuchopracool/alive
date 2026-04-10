#ifndef GUI_UTILITY_H
#define GUI_UTILITY_H

#include <QColor>
#include <QIcon>
#include <QPainter>

namespace alive::gui {
inline void draw_icon(QPainter *painter,
                      const QIcon &icon,
                      const QRect &icon_rect,
                      const QColor &background_color,
                      const QColor &outline_color)
{
    painter->save();
    painter->setBrush(background_color);
    painter->setPen(outline_color);
    painter->drawRect(icon_rect.adjusted(-1, -1, 1, 1));
    icon.paint(painter, icon_rect);
    painter->restore();
}
} // namespace alive::gui

#endif // GUI_UTILITY_H
