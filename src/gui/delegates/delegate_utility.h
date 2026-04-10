#ifndef DELEGATE_UTILITY_H
#define DELEGATE_UTILITY_H

#include <qglobal.h>

QT_BEGIN_NAMESPACE
class QRectF;
class QPointF;
QT_END_NAMESPACE

namespace alive {
enum class ClickPosition { e_None, e_First, e_Second, e_Third, e_Fourth };

ClickPosition left_icon_position_for_click(const QPointF &pos, const QRectF &rect);
ClickPosition right_icon_position_for_click(const QPointF &pos, const QRectF &rect);
ClickPosition column_position_for_click(const QPointF &pos, const QRectF &rect);
} // namespace alive

#endif // DELEGATE_UTILITY_H
