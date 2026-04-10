#include "color_palette.h"

namespace alive::theme {

namespace {
static ColorPalette *the_instance = nullptr;
}

struct ColorPaletteData
{
    std::vector<QColor> colors;
};

ColorPalette::ColorPalette(QObject *parent)
    : QAbstractListModel{parent}
    , m_d(std::make_unique<ColorPaletteData>())
{
    std::vector<QColor> &colors = m_d->colors;
    colors.resize(e_Last);
    colors[e_BackgroundPrimary] = QColor{35, 35, 35};
    colors[e_BackgroundSecondary] = QColor{55, 55, 55};
    colors[e_PopupBackground] = QColor{60, 60, 60};
    colors[e_TextField] = QColor{20, 20, 20};
    colors[e_Button] = QColor{85, 85, 85};
    colors[e_FontPrimary] = QColor{200, 200, 200};
    colors[e_FontSecondary] = QColor{150, 150, 150};
    colors[e_Bright] = QColor{230, 230, 230};
    colors[e_Dark] = QColor{0, 0, 0};
    colors[e_Stroke] = QColor{5, 5, 5};
    colors[e_Link] = QColor{45, 140, 240};
    colors[e_Accent] = QColor{45, 140, 240}; //{255, 197, 122};
    colors[e_PlayCursorColor] = QColor{255, 0, 0};
}

ColorPalette::~ColorPalette() {}

const QColor &ColorPalette::get_color(Color index) const
{
    Q_ASSERT(index < e_Last);
    return m_d->colors[index];
}

const QColor &ColorPalette::primary_font_color() const
{
    return m_d->colors[e_FontPrimary];
}

const QColor &ColorPalette::secondary_font_color() const
{
    return m_d->colors[e_FontSecondary];
}

const QColor &ColorPalette::bright_color() const
{
    return m_d->colors[e_Bright];
}

const QColor &ColorPalette::dark_color() const
{
    return m_d->colors[e_Dark];
}

const QColor &ColorPalette::accent_color() const
{
    return m_d->colors[e_Accent];
}

const QColor ColorPalette::transparent_accent_color() const
{
    QColor color = accent_color();
    color.setAlpha(35);
    return color;
}

const QColor &ColorPalette::primary_background_color() const
{
    return m_d->colors[e_BackgroundPrimary];
}

const QColor &ColorPalette::secondary_background_color() const
{
    return m_d->colors[e_BackgroundSecondary];
}

const QColor &ColorPalette::link_color() const
{
    return m_d->colors[e_Link];
}

const QColor &ColorPalette::stroke_color() const
{
    return m_d->colors[e_Stroke];
}

const QColor &ColorPalette::button_color() const
{
    return m_d->colors[e_Button];
}

const QColor &ColorPalette::popup_color() const
{
    return m_d->colors[e_PopupBackground];
}

const QColor &ColorPalette::play_cursor_color() const
{
    return m_d->colors[e_PlayCursorColor];
}

int ColorPalette::rowCount(const QModelIndex &parent) const
{
    return e_Last;
}

QVariant ColorPalette::data(const QModelIndex &index, int role) const
{
    if (index.row() < e_Last && role == Qt::DisplayRole) {
        return m_d->colors[index.row()].name();
    }
    return {};
}

void ColorPalette::init()
{
    if (!the_instance) {
        the_instance = new ColorPalette();
    }
}

void ColorPalette::deinit(bool qml)
{
    if (!qml && the_instance) {
        delete the_instance;
        the_instance = nullptr;
    }
}

ColorPalette *ColorPalette::instance()
{
    return the_instance;
}
} // namespace alive::theme
