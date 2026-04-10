#include "keyframe_item.h"
#include <core/model/property/property.h>
#include <core/utility.h>
#include <gui/document/composition_document.h>
#include <core/model/property/easing.h>
#include <gui/quick/range_selector_helper.h>
#include <gui/framework/theme/color_palette.h>
#include <gui/theme/timeline_style.h>
#include <gui/utility/keyframe_gui_utility.h>
#include <QMouseEvent>
#include <QPainter>

namespace alive::gui {
KeyFrameItem::KeyFrameItem(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
    setAcceptedMouseButtons(Qt::AllButtons);
}

KeyFrameItem::~KeyFrameItem() {}

void KeyFrameItem::paint(QPainter *painter)
{
    if (m_property && m_helper && m_property->is_animating()) {
        painter->setBrush(alive::theme::ColorPalette::instance()->button_color());
        QColor selection_color = alive::theme::ColorPalette::instance()->link_color();
        QColor playhead_color = alive::theme::ColorPalette::instance()->play_cursor_color();
        qreal y = this->boundingRect().center().y();

        if (m_bounds.first && m_bounds.second) {
            qreal x1 = m_helper->position_for_frame(m_bounds.first->time());
            qreal x2 = m_helper->position_for_frame(m_bounds.second->time());
            QRectF rect(QPointF{x1, y - 2}, QPointF(x2, y + 2));
            painter->save();
            painter->setPen(Qt::NoPen);
            painter->setBrush(alive::theme::ColorPalette::instance()->accent_color());
            painter->drawRect(rect);
            painter->restore();
        }
        for (auto &[keyframe, status] : m_keyframes) {
            qreal frame_x = m_helper->position_for_frame(keyframe->time());
            painter->save();
            painter->translate(frame_x, y);
            bool scripted = keyframe->expression_enabled() && !keyframe->expression().empty();
            if (status.selected) {
                if (scripted) {
                    painter->setBrush(playhead_color);
                } else {
                    painter->setBrush(selection_color);
                }
            } else {
                if (scripted) {
                    painter->setBrush(playhead_color.lighter());
                }
            }
            painter->drawPath(alive::keyFrameGuiUtility::path_for_keyframe(keyframe));
            painter->restore();
        }
    }
}

void KeyFrameItem::set_item(TreeItem *item)
{
    if (item) {
        model::Property *property = item->to_property();
        if (m_property) {
            stop_observing(m_property);
        }

        m_property = property;
        if (m_property) {
            reset_keyframes();
            observe(m_property);
        }
        update();
    }
}

void KeyFrameItem::set_timeline_helper(TimelineHelper *helper)
{
    m_helper = helper;
    this->disconnect();
    if (m_helper) {
        connect(helper, &TimelineHelper::range_changed, this, [this]() { update(); });
        connect(this,
                &KeyFrameItem::add_keyframe_to_selection,
                helper,
                &TimelineHelper::slot_add_keyframe_to_selection);
        connect(this,
                &KeyFrameItem::clear_all_keyframes_from_selection,
                helper,
                &TimelineHelper::slot_clear_all_keyframes_from_selection);

        connect(this,
                &KeyFrameItem::add_only_one_keyframe_to_selection,
                helper,
                &TimelineHelper::slot_add_only_one_keyframe_to_selection);
        connect(this,
                &KeyFrameItem::remove_keyframe_from_selection,
                helper,
                &TimelineHelper::slot_remove_keyframe_from_selection);
        connect(helper,
                &TimelineHelper::clear_all_keyframes_from_selection,
                this,
                &KeyFrameItem::slot_clear_selection);
    }
    update();
}

void KeyFrameItem::on_update(model::ISubject *)
{
    reset_keyframes();
    update();
}

void KeyFrameItem::mousePressEvent(QMouseEvent *event)
{
    QPointF drag_start_pos = event->position();
    if (event->modifiers() == Qt::ShiftModifier && toggle_keyframe_selection(drag_start_pos)) {
    } else {
        bool clear_bound_selection = true;
        if (select_one_keyframe(drag_start_pos)) {
        } else {
            // if between two keyframes, give some hint
            if (m_keyframes.size() >= 2) {
                qreal y = event->pos().y();
                auto rect = this->boundingRect();
                auto y_top = rect.top();
                auto y_bottom = rect.bottom();
                if (alive::in_bound(y_top, y_bottom, y)) {
                    auto bounds = find_keyframes_around(event->pos());
                    clear_bound_selection = false;
                    if (m_bounds == bounds) {
                        if (event->modifiers() == Qt::ControlModifier) {
                            model::EasingBetweenTwoFramesData data(m_bounds.first, m_bounds.second);
                            data.m_ct1.x() = 0;
                            data.m_ct1.y() = 1;

                            data.m_ct2.x() = 0;
                            data.m_ct2.y() = 1;
                            const_cast<model::CompositionDocument *>(m_helper->document())
                                ->slot_edited_easing_between(data);

                        } else if (event->modifiers() == Qt::ShiftModifier) {
                            // model::EasingBetweenTwoFramesData data(m_property,
                            //                                        m_bounds.first,
                            //                                        m_bounds.second,
                            //                                        m_helper->position_for_frame(
                            //                                            m_bounds.first->time()),
                            //                                        m_helper->position_for_frame(
                            //                                            m_bounds.second->time()));
                            // QVariant var_data;
                            // var_data.setValue(data);
                            // emit edit_easing_between(var_data);
                        }
                    } else {
                        m_bounds = bounds;
                    }
                    update();
                }
            }
        }
        if (clear_bound_selection) {
            clear_bounds();
        }
    }
    m_drag_start_pos_x = drag_start_pos.x();
    m_dragging = true;
}

void KeyFrameItem::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging) {
        qreal delta_x = event->position().x() - m_drag_start_pos_x;
        bool negative = false;
        if (delta_x < 0) {
            negative = true;
            delta_x *= -1;
        }
        if (delta_x > m_helper->width_per_frame()) {
            int num_frames_moved = delta_x / m_helper->width_per_frame();
            if (negative) {
                num_frames_moved *= -1;
            }
            m_drag_start_pos_x += (num_frames_moved * m_helper->width_per_frame());
            m_helper->slot_frames_moved(num_frames_moved);
        }
    }
}

void KeyFrameItem::mouseReleaseEvent(QMouseEvent *event)
{
    m_dragging = false;
}

void KeyFrameItem::slot_clear_selection()
{
    if (!m_sent_clear_selection) {
        if (clear_selection()) {
            update();
        }
        clear_bounds();
    }
}

void KeyFrameItem::reset_keyframes()
{
    auto old_status = m_keyframes;
    m_keyframes.clear();
    if (m_property) {
        for (model::KeyFrame *keyframe : m_property->get_keyframes()) {
            m_keyframes[keyframe] = old_status[keyframe];
        }
    }
}

bool KeyFrameItem::select_one_keyframe(QPointF pos)
{
    model::KeyFrame *selected_keyframe = find_keyframe(pos);
    if (select_one_keyframe(selected_keyframe)) {
        update();
    }

    return selected_keyframe != nullptr;
}

bool KeyFrameItem::clear_selection()
{
    bool selected = false;
    for (auto &[keyframe, status] : m_keyframes) {
        selected |= status.selected;
        status.selected = false;
    }
    return selected;
}

bool KeyFrameItem::select_one_keyframe(model::KeyFrame *keyframe)
{
    QList<model::KeyFrame *> selected_keyframes = selected_frames();
    bool changed = false;
    bool was_selected = selected_keyframes.indexOf(keyframe) != -1;
    if (!was_selected) {
        for (auto &key : selected_keyframes) {
            m_keyframes[key].selected = false;
            changed = true;
        }

        m_sent_clear_selection = true;
        if (keyframe) {
            m_keyframes[keyframe].selected = true;
            emit add_only_one_keyframe_to_selection(keyframe);
        } else /*if (changed && selected_keyframes.size())*/ {
            emit clear_all_keyframes_from_selection();
        }
        m_sent_clear_selection = false;
    }

    return changed || (!was_selected);
}

bool KeyFrameItem::toggle_keyframe_selection(QPointF pos)
{
    model::KeyFrame *keyframe = find_keyframe(pos);
    if (keyframe) {
        bool &selected = m_keyframes[keyframe].selected;
        if (selected) {
            emit remove_keyframe_from_selection(keyframe);
        } else {
            emit add_keyframe_to_selection(keyframe);
        }
        selected = !selected;
        update();
        return true;
    }
    return false;
}

model::KeyFrame *KeyFrameItem::find_keyframe(QPointF pos)
{
    model::KeyFrame *found = nullptr;
    qreal y = this->boundingRect().center().y();
    auto it = m_keyframes.begin();
    while (it != m_keyframes.end()) {
        model::KeyFrame *keyframe = it->first;
        qreal frame_x = m_helper->position_for_frame(keyframe->time());
        QPointF pos_to_check(pos.x() - frame_x, pos.y() - y);
        if (alive::keyFrameGuiUtility::path_for_keyframe(keyframe).contains(pos_to_check)) {
            found = keyframe;
            break;
        }
        ++it;
    }
    return found;
}

std::pair<model::KeyFrame *, model::KeyFrame *> KeyFrameItem::find_keyframes_around(QPointF pos)
{
    std::pair<model::KeyFrame *, model::KeyFrame *> result;
    result.first = nullptr;
    result.second = nullptr;

    if (m_keyframes.size() < 2)
        return result;

    auto it = m_keyframes.begin();
    model::KeyFrame *prev_keyframe = nullptr;
    while (it != m_keyframes.end()) {
        model::KeyFrame *keyframe = it->first;
        qreal frame_x = m_helper->position_for_frame(keyframe->time());
        if (frame_x > pos.x()) {
            result.first = prev_keyframe;
            result.second = keyframe;
            break;
        }
        prev_keyframe = keyframe;
        ++it;
    }
    return result;
}

QList<model::KeyFrame *> KeyFrameItem::selected_frames() const
{
    QList<model::KeyFrame *> result;
    auto it = m_keyframes.begin();
    while (it != m_keyframes.end()) {
        if (it->second.selected) {
            result.append(it->first);
        }
        ++it;
    }
    return result;
}

void KeyFrameItem::clear_bounds()
{
    if (m_bounds.first || m_bounds.second) {
        m_bounds.first = nullptr;
        m_bounds.second = nullptr;
        update();
    }
}

bool CompareKeyframes::operator()(const model::KeyFrame *v1, const model::KeyFrame *v2) const
{
    return v1->time() < v2->time();
}
} // namespace alive::gui
