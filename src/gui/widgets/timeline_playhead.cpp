#include "timeline_playhead.h"
#include <gui/framework/theme/color_palette.h>
#include <gui/theme/timeline_style.h>
#include <QPainter>
#include <QStyleOption>

namespace alive {

TimelinePlayHead::TimelinePlayHead(QWidget *parent, Type type)
    : QWidget{parent}
    , m_type(type)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
    if (m_type == e_Selected) {
        m_playhead_icon = QIcon::fromTheme("playhead");
    }
}

void TimelinePlayHead::paintEvent(QPaintEvent *ev)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    const auto *color_theme = alive::theme::ColorPalette::instance();
    QPen pen;
    pen.setWidth(1);
    const auto *sizes = alive::theme::TimelineSize::instance();
    QPoint top_mid(width() / 2, 0);
    QPoint bottom_mid(width() / 2, height());
    if (m_type == e_Selected) {
        pen.setColor(color_theme->accent_color());
        p.setPen(pen);
        QPoint l1_bottom(width() / 2, sizes->visible_range_selector_height());
        QPoint l2_top(width() / 2, sizes->visible_range_selector_height() + sizes->ruler_height());
        p.drawLine(top_mid, l1_bottom);
        p.drawLine(l2_top, bottom_mid);

        QRect rect(0, 0, sizes->playhead_size().width(), sizes->playhead_size().height());
        rect.moveBottom(sizes->visible_range_selector_height() + sizes->ruler_height());
        m_playhead_icon.paint(&p, rect);
    } else {
        pen.setColor(color_theme->play_cursor_color());
        p.setPen(pen);
        p.drawLine(top_mid, bottom_mid);
    }
}
} // namespace alive
