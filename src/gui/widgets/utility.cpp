#include "utility.h"

#include <gui/icon_manager.h>
#include <gui/framework/theme/color_palette.h>
#include <gui/theme/timeline_style.h>

#include <QHBoxLayout>
#include <QPainter>
#include <QRect>
#include <QVBoxLayout>

namespace alive {

QHBoxLayout *hbox_layout(int margin)
{
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(margin, margin, margin, margin);
    layout->setSpacing(0);
    return layout;
}

QVBoxLayout *vbox_layout(int margin)
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(margin, margin, margin, margin);
    layout->setSpacing(0);
    return layout;
}

void draw_combobox_from_item_view(QPainter *painter,
                                  const QString &text,
                                  const QRect &rect,
                                  const alive::theme::ColorPalette *colors,
                                  const alive::theme::TimelineSize *sizes,
                                  const IconManager *icon_mgr)
{
    QRect text_rect(rect.adjusted(1, 1, -1, -1));
    painter->save();
    painter->setPen(colors->primary_font_color());
    painter->drawRect(text_rect);
    painter->drawText(rect.adjusted(sizes->item_spacing(), 0, -sizes->icon_size().width(), 0),
                      text,
                      Qt::AlignLeft | Qt::AlignVCenter);
    QRect icon_rect(0, 0, sizes->icon_size().width(), sizes->icon_size().height());
    QPoint p(text_rect.right() - icon_rect.width() / 2, text_rect.center().y());
    icon_rect.moveLeft(-icon_rect.width() / 2);
    icon_rect.moveTop(-icon_rect.height() / 2);
    painter->translate(p);
    painter->rotate(90);
    icon_mgr->get_icon(IconManager::e_ComboDownArrow).paint(painter, icon_rect);
    painter->restore();
}

} // namespace alive
