#include "delegate_utility.h"
#include <gui/theme/timeline_style.h>
#include <QRect>
#include <QStyleOption>

namespace alive {

ClickPosition left_icon_position_for_click(const QPointF &pos, const QRectF &rect)
{
    const auto *sizes = alive::theme::TimelineSize::instance();
    const auto &icon_size = sizes->icon_size();
    QRectF icon_rect(0, 0, icon_size.width(), icon_size.height());
    QPointF new_center(rect.left() + icon_rect.width() / 2 + sizes->item_spacing(),
                       rect.center().y());
    icon_rect.moveCenter(new_center);
    if (icon_rect.contains(pos)) {
        return ClickPosition::e_First;
    }

    icon_rect.moveLeft(icon_rect.right() + sizes->item_spacing());
    if (icon_rect.contains(pos)) {
        return ClickPosition::e_Second;
    }

    icon_rect.moveLeft(icon_rect.right() + sizes->item_spacing());
    if (icon_rect.contains(pos)) {
        return ClickPosition::e_Third;
    }

    return ClickPosition::e_None;
}

ClickPosition right_icon_position_for_click(const QPointF &pos, const QRectF &rect)
{
    const auto *sizes = alive::theme::TimelineSize::instance();
    const auto &icon_size = sizes->icon_size();
    QRectF icon_rect(0, 0, icon_size.width(), icon_size.height());
    QPointF new_center(rect.right() - icon_rect.width() / 2 - sizes->item_spacing(),
                       rect.center().y());
    icon_rect.moveCenter(new_center);
    if (icon_rect.contains(pos)) {
        return ClickPosition::e_First;
    }

    icon_rect.moveRight(icon_rect.left() - sizes->item_spacing());
    if (icon_rect.contains(pos)) {
        return ClickPosition::e_Second;
    }

    icon_rect.moveRight(icon_rect.left() - sizes->item_spacing());
    if (icon_rect.contains(pos)) {
        return ClickPosition::e_Third;
    }

    return ClickPosition::e_None;
}
ClickPosition column_position_for_click(const QPointF &pos, const QRectF &rect)
{
    const auto *sizes = alive::theme::TimelineSize::instance();
    QRectF label_rect(0, rect.top(), sizes->column_width(), rect.height());
    label_rect.moveLeft(rect.left());
    if (label_rect.contains(pos)) {
        return ClickPosition::e_First;
    }

    label_rect.moveLeft(label_rect.right());
    if (label_rect.contains(pos)) {
        return ClickPosition::e_Second;
    }

    label_rect.moveLeft(label_rect.right());
    if (label_rect.contains(pos)) {
        return ClickPosition::e_Third;
    }

    label_rect.moveLeft(label_rect.right());
    if (label_rect.contains(pos)) {
        return ClickPosition::e_Fourth;
    }

    return ClickPosition::e_None;
}

} // namespace alive
