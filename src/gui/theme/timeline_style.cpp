#include "timeline_style.h"
#include <gui/framework/theme/color_palette.h>
#include <QPainter>

// QPixmap alive::theme::TimelineStyle::handle_pixmap(int height,
//                                                   qreal device_pixel_ratio,
//                                                   bool left) const
// {
//     QSize handle_size(range_selector_handle_width, height);
//     handle_size *= device_pixel_ratio;

//     QPixmap result(handle_size);
//     result.fill(Qt::transparent);
//     QPainter painter(&result);
//     painter.setPen(Qt::NoPen);
//     painter.setRenderHint(QPainter::Antialiasing);

//     int rect_width = handle_size.width() - outline_color_width;
//     int rect_height = handle_size.height() - outline_color_width;
//     int radius = (range_selector_handle_radius * device_pixel_ratio);
//     radius = std::min(std::min(rect_width, rect_height / 2), radius);
//     painter.setBrush(
//         theme::ApplicationStyle::instance().colors().get_color(theme::ColorPalette::e_Link));
//     painter.drawRoundedRect(QRect(0, outline_color_width / 2, rect_width, rect_height),
//                             radius,
//                             radius);

//     int rect_x = left ? rect_width / 2 : 0;
//     painter.drawRect(rect_x, outline_color_width / 2, rect_width / 2, rect_height);
//     return result;
// }
namespace {
static alive::theme::TimelineSize *the_instance = nullptr;
} // namespace

void alive::theme::TimelineSize::init()
{
    if (!the_instance) {
        the_instance = new alive::theme::TimelineSize();
    }
}

void alive::theme::TimelineSize::deinit(bool qml)
{
    if (!qml && the_instance) {
        delete the_instance;
        the_instance = nullptr;
    }
}

alive::theme::TimelineSize *alive::theme::TimelineSize::instance()
{
    return the_instance;
}

alive::theme::TimelineSize::TimelineSize(QObject *parent)
    : QObject(parent)
{}
