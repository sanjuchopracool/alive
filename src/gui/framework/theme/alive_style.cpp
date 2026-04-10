#include "alive_style.h"
#include <gui/framework/theme/color_palette.h>
#include <gui/theme/timeline_style.h>
#include <QPainter>
#include <QStyleOption>

namespace alive::theme {

Style::Style(QStyle *style)
    : QProxyStyle(style)
{}
namespace {
void draw_splitter(const QStyleOption *option, QPainter *painter)
{
    painter->save();
    QPoint p1;
    QPoint p2;
    if (option->state & QStyle::State_Horizontal) {
        p1 = QPoint(option->rect.center().x(), option->rect.top());
        p2 = QPoint(option->rect.center().x(), option->rect.bottom());
    } else {
        p1 = QPoint(option->rect.left(), option->rect.center().y());
        p2 = QPoint(option->rect.right(), option->rect.center().y());
    }
    if (option->state & QStyle::QStyle::State_MouseOver) {
        painter->setPen(alive::theme::ColorPalette::instance()->bright_color());
    } else {
        painter->setPen(alive::theme::ColorPalette::instance()->stroke_color());
    }

    painter->drawLine(p1, p2);
    painter->restore();
}
} // namespace

void Style::polish(QPalette &pl)
{
    auto *colors = alive::theme::ColorPalette::instance();
    // Active Group
    pl.setCurrentColorGroup(QPalette::Active);
    pl.setColor(QPalette::Base, colors->secondary_background_color());
    pl.setColor(QPalette::Window, colors->primary_background_color());
    pl.setColor(QPalette::Button, colors->primary_background_color());
    pl.setColor(QPalette::WindowText, colors->primary_font_color());
    pl.setColor(QPalette::Text, colors->primary_font_color());
    pl.setColor(QPalette::ButtonText, colors->primary_font_color());
    pl.setColor(QPalette::PlaceholderText, colors->secondary_font_color());
    pl.setColor(QPalette::Dark, colors->dark_color());

    auto print_colors = [&pl]() {
        for (auto i = QPalette::WindowText; i < QPalette::NColorRoles;) {
            qDebug() << i << " : " << pl.color(i).name();
            i = static_cast<QPalette::ColorRole>(i + 1);
        }
    };

    qDebug() << "Active Group";
    print_colors();
    //Disabled Group
    pl.setCurrentColorGroup(QPalette::Disabled);
    qDebug() << "Disabled Group";
    print_colors();
}

void Style::drawControl(ControlElement element,
                        const QStyleOption *option,
                        QPainter *painter,
                        const QWidget *widget) const
{
    switch (element) {
    case ControlElement::CE_Splitter:
        draw_splitter(option, painter);
        break;
    default:
        QProxyStyle::drawControl(element, option, painter, widget);
    }
}

} // namespace alive::theme
