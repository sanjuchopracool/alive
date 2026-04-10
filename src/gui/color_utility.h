#ifndef COLOR_UTILITY_H
#define COLOR_UTILITY_H
#include <core/alive_types/color_type.h>
#include <core/alive_types/types.h>
#include <qcolor.h>

namespace alive::gui {
inline Color4ub to_sk_color(const QColor &color)
{
    return Color4ub(color.red(), color.green(), color.blue(), color.alpha());
}
inline QColor from_sk_color(const Color4ub &color)
{
    return QColor(color.r(), color.g(), color.b(), color.a());
}

inline QColor from_color(const Color &color)
{
    return QColor::fromRgbF(color.r(), color.g(), color.b());
}

inline Color to_color(const QColor &color)
{
    return Color(color.redF(), color.greenF(), color.blueF());
}

inline QColor from_color4ub(const Color4ub &color)
{
    return QColor(color.r(), color.g(), color.b(), color.a());
}

inline Color4ub to_color4ub(const QColor &color)
{
    return Color4ub(color.red(), color.green(), color.blue(), color.alpha());
}

inline QColor from_rgba_string(const QString &str)
{
    uint32_t color = static_cast<uint32_t>(
        std::stoll("0x" + str.toStdString().substr(1), nullptr, 16));
    uint8_t r = (0xFF000000 & color) >> 24;
    uint8_t g = (0x00FF0000 & color) >> 16;
    uint8_t b = (0x0000FF00 & color) >> 8;
    uint8_t a = (0x000000FF & color);
    return QColor{r, g, b, a};
}
inline QColor from_rgb_string(const QString &str)
{
    uint32_t color = static_cast<uint32_t>(
        std::stoll("0x" + str.toStdString().substr(1), nullptr, 16));
    uint8_t r = (0x00FF0000 & color) >> 16;
    uint8_t g = (0x0000FF00 & color) >> 8;
    uint8_t b = (0x000000FF & color);
    return QColor{r, g, b, 255};
}

inline QString to_rgba_string(QColor color)
{
    return QString::fromStdString(color_to_rgba_string(to_color4ub(color)));
}

} // namespace alive::gui
#endif // COLOR_UTILITY_H
