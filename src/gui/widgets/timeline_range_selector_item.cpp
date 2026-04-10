#include "timeline_range_selector_item.h"
#include <gui/composition/timeline.h>
#include <gui/cursor_manager.h>
#include <gui/quick/range_selector_helper.h>
#include <gui/framework/theme/color_palette.h>
#include <gui/theme/timeline_style.h>

#include <QEnterEvent>
#include <QPainter>

namespace alive {
TimelineRangeSelectorItem::TimelineRangeSelectorItem(alive::gui::TimelineHelper *helper,
                                                     SelectionType type,
                                                     QWidget *parent)
    : QWidget{parent}
    , m_helper(helper)
    , m_type(type)
{
    setMouseTracking(true);
    auto *timeline = helper->timeline();
    connect(timeline, &alive::gui::TimeLine::play_range_changed, this, [this]() { update(); });
    connect(timeline, &alive::gui::TimeLine::range_changed, this, [this]() { update(); });
}

QSize TimelineRangeSelectorItem::sizeHint() const
{
    return QSize(400, 100);
}

TimelineRangeSelectorItem::~TimelineRangeSelectorItem() {}

void TimelineRangeSelectorItem::paintEvent(QPaintEvent *ev)
{
    Q_UNUSED(ev)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    const auto *color_theme = alive::theme::ColorPalette::instance();
    const auto *timeline_style = alive::theme::TimelineSize::instance();
    // Draw background
    painter.fillRect(this->rect(), color_theme->primary_background_color());

    QPen pen(color_theme->stroke_color(), timeline_style->stroke_width());
    painter.setPen(pen);

    auto *timeline = m_helper->timeline();
    int left_handle_right = 0;
    int right_handle_left = 0;
    // draw play range
    {
        painter.save();
        if (m_type == e_VisibleRange) {
            painter.setBrush(color_theme->secondary_background_color());
            painter.drawRect(m_helper->visible_play_range_start_x(),
                             0,
                             m_helper->visible_play_range_width(),
                             height());
        } else {
            left_handle_right = m_helper->position_for_frame(timeline->play_start());
            right_handle_left = m_helper->position_for_frame(timeline->play_end());
            QRectF range_rect(left_handle_right, 0, right_handle_left - left_handle_right, height());
            painter.setBrush(color_theme->button_color());
            painter.drawRect(range_rect);
        }
        painter.restore();
    }

    // draw visible range
    if (m_type == e_VisibleRange) {
        painter.save();
        left_handle_right = m_helper->visible_range_start_x();
        right_handle_left = left_handle_right + m_helper->visible_range_width();
        painter.setBrush(color_theme->button_color());
        painter.drawRect(left_handle_right, 0, m_helper->visible_range_width(), height());
        painter.restore();
    }
    painter.setBrush(color_theme->link_color());

    // draw left handle
    const int handle_width = timeline_style->handle_width();
    painter.save();
    painter.translate(left_handle_right - handle_width, 0);
    painter.drawPath(m_handle_path);
    painter.restore();

    // draw right handle
    painter.translate(right_handle_left + handle_width, height());
    painter.rotate(180);
    painter.drawPath(m_handle_path);
}

void TimelineRangeSelectorItem::resizeEvent(QResizeEvent *ev)
{
    Q_UNUSED(ev)
    update_path();
}

void TimelineRangeSelectorItem::enterEvent(QEnterEvent *ev)
{
    update_cursor(find_overlap_area(ev->position().x()));
    QWidget::enterEvent(ev);
}

void TimelineRangeSelectorItem::leaveEvent(QEvent *ev)
{
    update_cursor(e_None);
    QWidget::leaveEvent(ev);
}

void TimelineRangeSelectorItem::mouseMoveEvent(QMouseEvent *ev)
{
    qreal x = ev->position().x();
    if (is_dragging()) {
        if (m_drag_area == e_Range) {
            if (m_type == e_VisibleRange) {
                m_helper->visible_range_moved(x);
            } else {
                m_helper->play_range_moved(x);
            }
        } else if (m_drag_area == e_LeftHandle) {
            if (m_type == e_VisibleRange) {
                m_helper->select_range_start_for_x(x);
            } else {
                m_helper->select_play_start_for_x(x);
            }
        } else if (m_drag_area == e_RightHandle) {
            if (m_type == e_VisibleRange) {
                m_helper->select_range_end_for_x(x);
            } else {
                m_helper->select_play_end_for_x(x);
            }
        }
    } else {
        update_cursor(find_overlap_area(x));
    }
    QWidget::mouseMoveEvent(ev);
}

void TimelineRangeSelectorItem::mousePressEvent(QMouseEvent *ev)
{
    qreal x = ev->position().x();
    m_drag_area = find_overlap_area(x);
    if (m_drag_area == e_Range) {
        if (m_type == e_VisibleRange) {
            m_helper->visible_range_move_started(x);
        } else {
            m_helper->play_range_move_started(x);
        }
    }
    QWidget::mousePressEvent(ev);
}

void TimelineRangeSelectorItem::mouseReleaseEvent(QMouseEvent *ev)
{
    m_drag_area = e_None;
    update_cursor(find_overlap_area(ev->position().x()));
    QWidget::mouseReleaseEvent(ev);
}

void TimelineRangeSelectorItem::update_path()
{
    // Left Handle
    const auto *timeline_style = alive::theme::TimelineSize::instance();
    const auto &border_radius = timeline_style->handle_radius();
    QRectF rect(0, 0, timeline_style->handle_width(), this->height());
    m_handle_path.clear();
    QRectF arc_rect({0, 0}, QSizeF(2 * border_radius, 2 * border_radius));
    m_handle_path.moveTo(rect.topRight());
    qreal mid_x = rect.left() + border_radius;
    m_handle_path.lineTo({mid_x, rect.top()});
    qreal mid_y_bottom = rect.bottom() - border_radius;
    arc_rect.moveLeft(rect.left());
    arc_rect.moveTop(rect.top());
    m_handle_path.arcTo(arc_rect, 90, 90);
    m_handle_path.lineTo({rect.left(), mid_y_bottom});
    arc_rect.moveBottom(rect.bottom());
    m_handle_path.arcTo(arc_rect, 180, 90);
    m_handle_path.lineTo({mid_x, rect.bottom()});
    m_handle_path.lineTo(rect.bottomRight());
    m_handle_path.closeSubpath();
}

TimelineRangeSelectorItem::MousePositionArea TimelineRangeSelectorItem::find_overlap_area(
    qreal x) const
{
    MousePositionArea result = TimelineRangeSelectorItem::e_None;
    qreal range_x1 = 0;
    qreal range_x2 = 0;
    if (m_type == e_VisibleRange) {
        range_x1 = m_helper->visible_range_start_x();
        range_x2 = range_x1 + m_helper->visible_range_width();
    } else {
        auto *timeline = m_helper->timeline();
        range_x1 = m_helper->position_for_frame(timeline->play_start());
        range_x2 = m_helper->position_for_frame(timeline->play_end());
    }

    const int handle_width = alive::theme::TimelineSize::instance()->handle_width();
    if (x >= range_x1 && x <= range_x2) {
        result = e_Range;
    } else if (x < range_x1 && (x >= (range_x1 - handle_width))) {
        result = e_LeftHandle;
    } else if (x > range_x2 && (x <= (handle_width + range_x2))) {
        result = e_RightHandle;
    }
    return result;
}

void TimelineRangeSelectorItem::update_cursor(MousePositionArea area)
{
    if (m_mouse_pos_area == area) {
        return;
    }

    auto cursor_manager = alive::gui::CursorManager::instance();
    if (area == e_None) {
        if (m_mouse_pos_area != e_None) {
            cursor_manager->restore_cursor(m_cursor_id);
        }
    } else {
        if (area == e_Range) {
            m_cursor_id = cursor_manager->horizontal_size_cursor();
        } else if (area == e_LeftHandle) {
            m_cursor_id = cursor_manager->left_drag_cursor();
        } else if (area == e_RightHandle) {
            m_cursor_id = cursor_manager->right_drag_cursor();
        }
    }
    m_mouse_pos_area = area;
}
} // namespace Alive
