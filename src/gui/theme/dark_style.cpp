#include "dark_style.h"

#include <theme/application_style.h>
#include <theme/color_palette.h>
#include <QStyleFactory>

namespace inae::theme {
DarkStyle::DarkStyle()
    : QProxyStyle(QStyleFactory::create("Fusion"))
{
    setObjectName("DarkStyle");
}

QPalette DarkStyle::standardPalette() const
{
    if (!m_standardPalette.isBrushSet(QPalette::Disabled, QPalette::Mid)) {
        const auto &colors = theme::ApplicationStyle::instance().colors();
        QColor background_color = colors.primary_background_color();
        QColor text_color = colors.primary_font_color();
        //        QColor slightlyOpaqueBlack(0, 0, 0, 63);

        //        QImage backgroundImage(":/images/woodbackground.png");
        //        QImage buttonImage(":/images/woodbutton.png");
        //        QImage midImage = buttonImage.convertToFormat(QImage::Format_RGB32);

        //        QPainter painter;
        //        painter.begin(&midImage);
        //        painter.setPen(Qt::NoPen);
        //        painter.fillRect(midImage.rect(), slightlyOpaqueBlack);
        //        painter.end();
        //! [0]

        //! [1]
        QPalette palette;
        palette.setBrush(QPalette::Window, background_color);
        palette.setBrush(QPalette::Base, background_color);
        //        palette.setBrush(QPalette::AlternateBase, background_color);
        palette.setBrush(QPalette::Button, background_color);
        palette.setBrush(QPalette::Text, text_color);
        palette.setBrush(QPalette::WindowText, text_color);
        palette.setBrush(QPalette::ButtonText, text_color);
        palette.setBrush(QPalette::Highlight, text_color);
        palette.setBrush(QPalette::HighlightedText, background_color);
        //        setTexture(palette, QPalette::Button, buttonImage);
        //        setTexture(palette, QPalette::Mid, midImage);
        //        setTexture(palette, QPalette::Window, backgroundImage);

        QBrush brush = palette.window();
        brush.setColor(brush.color().darker());

        palette.setBrush(QPalette::Disabled, QPalette::WindowText, brush);
        palette.setBrush(QPalette::Disabled, QPalette::Text, brush);
        palette.setBrush(QPalette::Disabled, QPalette::ButtonText, brush);
        palette.setBrush(QPalette::Disabled, QPalette::Base, brush);
        palette.setBrush(QPalette::Disabled, QPalette::Button, brush);
        palette.setBrush(QPalette::Disabled, QPalette::Mid, brush);

        m_standardPalette = palette;
    }

    return m_standardPalette;
}
} // namespace inae::theme
