#ifndef WIDGET_UTILITY_H
#define WIDGET_UTILITY_H

#include <QHBoxLayout>
#include <QVBoxLayout>

QT_BEGIN_NAMESPACE
class QHBoxLayout;
class QVBoxLayout;
class QPainter;
class QRect;
class QString;
QT_END_NAMESPACE

namespace alive::theme {
class ColorPalette;
class TimelineSize;
} // namespace alive::theme
namespace alive {
class IconManager;
QHBoxLayout *hbox_layout(int margin = 0);
QVBoxLayout *vbox_layout(int margin = 0);
void draw_combobox_from_item_view(QPainter *painter,
                                  const QString &text,
                                  const QRect &rect,
                                  const alive::theme::ColorPalette *colors,
                                  const alive::theme::TimelineSize *sizes,
                                  const IconManager *icon_mgr);
} // namespace Alive

#endif // WIDGET_UTILITY_H
