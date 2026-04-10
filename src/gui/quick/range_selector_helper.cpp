#include "range_selector_helper.h"
#include <core/model/property/object.h>
#include <gui/document/composition_document.h>
#include <gui/project/project_manager.h>
#include <gui/theme/timeline_style.h>
#include <editor/undo_support/document_undo_manager.h>

namespace alive::gui {

namespace {
constexpr int k_min_visible_frames = 5;
}
TimelineHelper::TimelineHelper(TimeLine *timeline, QObject *parent)
    : QObject(parent)
    , m_timeline(timeline)
{
    auto check_selected_frame_range = [this]() {
        qreal x = position_for_frame(m_timeline->current_frame());
        if (m_selected_frame_position != x) {
            m_selected_frame_position = x;
            emit selected_frame_position_changed();
        }

        x = range_position_for_frame(m_timeline->current_frame());
        if (m_selected_frame_range_position != x) {
            m_selected_frame_range_position = x;
            emit selected_frame_range_position_changed();
        }
    };

    connect(m_timeline, &TimeLine::selected_frame_changed, this, [=]() {
        check_selected_frame_range();
    });

    auto check_visible_play_range_changes = [this]() {
        const qreal start_x = range_position_for_frame(m_timeline->play_start());
        if (start_x != m_visible_play_range_start_x) {
            m_visible_play_range_start_x = start_x;
            emit visible_play_range_start_x_changed();
        }
        const qreal end_x = range_position_for_frame(m_timeline->play_end());
        qreal width = end_x - start_x;
        if (m_visible_play_range_width != width) {
            m_visible_play_range_width = width;
            emit visible_play_range_width_changed();
        }
    };

    auto check_play_range_changes = [=, this]() {
        const qreal start_x = position_for_frame(m_timeline->play_start());
        if (start_x != m_play_range_start_x) {
            m_play_range_start_x = start_x;
            emit play_range_start_x_changed();
        }
        const qreal end_x = position_for_frame(m_timeline->play_end());
        qreal width = end_x - start_x;
        //        qDebug() << "Play endx" << end_x << " Play width " << width;
        if (m_play_range_width != width) {
            m_play_range_width = width;
            emit play_range_width_changed();
        }
        check_visible_play_range_changes();
    };

    auto check_visible_range_changes = [this]() {
        const qreal start_x = range_position_for_frame(m_timeline->range_start());
        if (start_x != m_visible_range_start_x) {
            m_visible_range_start_x = start_x;
            emit visible_range_start_x_changed();
        }
        const qreal end_x = range_position_for_frame(m_timeline->range_end());
        qreal width = end_x - start_x;
        if (m_visible_range_width != width) {
            m_visible_range_width = width;
            emit visible_range_width_changed();
        }
    };

    connect(m_timeline, &TimeLine::range_changed, this, [=, this]() {
        check_visible_range_changes();
        check_play_range_changes();
        check_selected_frame_range();
    });

    connect(m_timeline, &TimeLine::play_range_changed, this, [=]() { check_play_range_changes(); });
    connect(m_timeline, &TimeLine::play_frame_changed, this, &TimelineHelper::play_frame_changed);
    connect(m_timeline, &TimeLine::run_state_changed, this, &TimelineHelper::runstate_changed);
    connect(m_timeline, &TimeLine::range_changed, this, &TimelineHelper::range_changed);
    connect(m_timeline, &TimeLine::fps_changed, this, &TimelineHelper::fps_changed);
    connect(m_timeline,
            &TimeLine::selected_frame_changed,
            this,
            &TimelineHelper::current_frame_changed);

    auto check_all_changed = [=]() {
        check_visible_range_changes();
        check_selected_frame_range();
        check_play_range_changes();
    };

    connect(this, &TimelineHelper::width_changed, this, [=] { check_all_changed(); });
}

void TimelineHelper::select_frame_for_x(qreal x) const
{
    int frame = frame_for_click(x);
    //    qDebug() << "Selected frame: " << frame;
    m_timeline->select_frame(frame);
}

void TimelineHelper::select_range_start_for_x(qreal x)
{
    int frame = frame_for_range_click(x);
    //    qDebug() << "Set" << x << frame;
    int end_frame = m_timeline->range_end();
    if ((end_frame - frame) >= k_min_visible_frames) {
        const_cast<model::CompositionDocument *>(m_timeline->document())->undo_manager()
            ->set_visible_range_area(m_timeline, frame, end_frame, true);
    }
}

void TimelineHelper::select_range_end_for_x(qreal x)
{
    int frame = frame_for_range_click(x);
    //    qDebug() << "Set" << x << frame;
    int start = m_timeline->range_start();
    if ((frame - start) >= k_min_visible_frames) {
        const_cast<model::CompositionDocument *>(m_timeline->document())->undo_manager()
            ->set_visible_range_area(m_timeline, start, frame, true);
    }
}

void TimelineHelper::visible_range_move_started(qreal x)
{
    m_visible_range_move_start = x;
}

void TimelineHelper::visible_range_moved(qreal x)
{
    qreal delta = x - m_visible_range_move_start;
    bool negaive = false;
    if (delta < 0) {
        negaive = true;
        delta *= -1;
    }
    const qreal size_per_frame = visible_range_size_per_frame();
    int frame_moved = static_cast<int>(delta / size_per_frame);
    if (frame_moved) {
        qreal diff = delta - frame_moved * size_per_frame;
        if (negaive) {
            diff *= -1;
            frame_moved *= -1;
        }
        int new_start = m_timeline->range_start() + frame_moved;
        if (new_start < m_timeline->comp_start()) {
            return;
        }
        int new_end = m_timeline->range_end() + frame_moved;
        if (new_end > m_timeline->comp_end()) {
            return;
        }
        m_visible_range_move_start = x - diff;
        const_cast<model::CompositionDocument *>(m_timeline->document())->undo_manager()
            ->set_visible_range_area(m_timeline, new_start, new_end, true);
    }
}

void TimelineHelper::select_play_start_for_x(qreal x)
{
    int frame = frame_for_click(x);
    int end_frame = m_timeline->play_end();
    if ((end_frame - frame) >= k_min_visible_frames) {
        const_cast<model::CompositionDocument *>(m_timeline->document())->undo_manager()
            ->set_work_area_start(m_timeline, frame, true);
    }
}

void TimelineHelper::select_play_end_for_x(qreal x)
{
    int frame = m_timeline->play_start();
    int end_frame = frame_for_click(x);
    if ((end_frame - frame) >= k_min_visible_frames) {
        const_cast<model::CompositionDocument *>(m_timeline->document())->undo_manager()
            ->set_work_area_end(m_timeline, end_frame, true);
    }
}

void TimelineHelper::play_range_move_started(qreal x)
{
    m_play_range_move_start = x;
}

void TimelineHelper::play_range_moved(qreal x)
{
    qreal delta = x - m_play_range_move_start;
    bool negative = false;
    if (delta < 0) {
        negative = true;
        delta *= -1;
    }
    const qreal size_per_frame = play_range_size_per_frame();
    int frame_moved = static_cast<int>(delta / size_per_frame);
    if (frame_moved) {
        qreal diff = delta - frame_moved * size_per_frame;
        if (negative) {
            diff *= -1;
            frame_moved *= -1;
        }
        int new_start = m_timeline->play_start() + frame_moved;
        if (new_start < m_timeline->range_start()) {
            return;
        }
        int new_end = m_timeline->play_end() + frame_moved;
        if (new_end > m_timeline->range_end()) {
            return;
        }
        m_play_range_move_start = x - diff;
        m_timeline->set_play_range(new_start, new_end);
        const_cast<model::CompositionDocument *>(m_timeline->document())->undo_manager()
            ->set_work_area(m_timeline, new_start, new_end, true);
    }
}

int TimelineHelper::frame_for_click(qreal x) const
{
    const int range = m_timeline->range_end() - m_timeline->range_start();
    const int handle_width = alive::theme::TimelineSize::instance()->handle_width();
    const int effective_width = width() - 2 * handle_width;
    const float frame_approx = (range * (x - handle_width)) / effective_width;
    int frame = m_timeline->range_start() + std::round(frame_approx);
    if (frame < m_timeline->range_start())
        frame = m_timeline->range_start();

    if (frame >= m_timeline->range_end())
        frame = m_timeline->range_end();

    return frame;
}

qreal TimelineHelper::position_for_frame(int frame) const
{
    //    qDebug() << "Widget width " << width();
    //    qDebug() << "timeline start " << m_timeline->range_start();
    //    qDebug() << "timeline end " << m_timeline->range_end();
    qreal x = 0;
    const int handle_width = alive::theme::TimelineSize::instance()->handle_width();
    const qreal effective_width = width() - 2 * handle_width;
    const int range_start = m_timeline->range_start();
    const int range_end = m_timeline->range_end();
    //    if (frame >= range_start && frame <= range_end) {
    const int range = range_end - range_start;
    x = (((frame - range_start) * effective_width) / range) + handle_width;
    //    }
    return x;
}

void TimelineHelper::select_current_frame(KeyFrameTime frame) const
{
    if (m_timeline) {
        return m_timeline->select_frame(frame);
    }
}

int TimelineHelper::current_frame() const
{
    if (m_timeline) {
        return m_timeline->current_frame();
    }
    return 0;
}

int TimelineHelper::play_frame() const
{
    if (m_timeline) {
        return m_timeline->play_frame();
    }
    return 0;
}

bool TimelineHelper::is_running() const
{
    if (m_timeline) {
        return m_timeline->is_running();
    }
    return false;
}

int TimelineHelper::range_start() const
{
    if (m_timeline) {
        return m_timeline->range_start();
    }
    return 0;
}

int TimelineHelper::range_end() const
{
    if (m_timeline) {
        return m_timeline->range_end();
    }
    return 30;
}

qreal TimelineHelper::fps() const
{
    if (m_timeline) {
        return m_timeline->fps();
    }
    return 30;
}

void TimelineHelper::set_width(qreal width)
{
    if (m_width != width) {
        m_width = width;

        const int range = m_timeline->range_end() - m_timeline->range_start();
        const int handle_width = alive::theme::TimelineSize::instance()->handle_width();
        qreal effective_width = this->width() - 2 * handle_width;
        m_width_per_frame = effective_width / range;
        emit width_changed();
    }
}

const model::CompositionDocument *TimelineHelper::document() const
{
    return m_timeline->document();
}

void TimelineHelper::slot_clear_all_keyframes_from_selection()
{
    emit clear_all_keyframes_from_selection();
    m_selected_keyframes.clear();
}

void TimelineHelper::slot_add_keyframe_to_selection(model::KeyFrame *keyframe)
{
    m_selected_keyframes.emplace(keyframe);
}

void TimelineHelper::slot_remove_keyframe_from_selection(model::KeyFrame *keyframe)
{
    m_selected_keyframes.erase(keyframe);
}

void TimelineHelper::slot_add_only_one_keyframe_to_selection(model::KeyFrame *keyframe)
{
    emit clear_all_keyframes_from_selection();
    m_selected_keyframes.clear();
    m_selected_keyframes.emplace(keyframe);
}

void TimelineHelper::slot_frames_moved(KeyFrameTime frames)
{
    if (!m_selected_keyframes.empty()) {
        model::Property *property = (*m_selected_keyframes.begin())->property();
        if (property) {
            model::CompositionDocument *doc = const_cast<model::CompositionDocument *>(
                m_timeline->document());
            if (doc) {
                doc->move_keyframes(m_selected_keyframes, frames);
            }
        }
    }
}

int TimelineHelper::frame_for_range_click(qreal x) const
{
    const int range = m_timeline->comp_end() - m_timeline->comp_start();
    const int handle_width = alive::theme::TimelineSize::instance()->handle_width();
    const qreal effective_width = width() - 2 * handle_width;
    const float frame_approx = (range * (x - handle_width)) / effective_width;
    int frame = m_timeline->comp_start() + std::round(frame_approx);
    if (frame < m_timeline->comp_start())
        frame = m_timeline->comp_start();

    if (frame >= m_timeline->comp_end())
        frame = m_timeline->comp_end();

    return frame;
}

qreal TimelineHelper::range_position_for_frame(int frame) const
{
    qreal x = 0;
    const int handle_width = alive::theme::TimelineSize::instance()->handle_width();
    const qreal effective_width = width() - 2 * handle_width;
    const int range_start = m_timeline->comp_start();
    const int range_end = m_timeline->comp_end();
    const int range = range_end - range_start;
    if (frame >= range_start && frame <= range_end) {
        x = (((frame - range_start) * effective_width) / range) + handle_width;
    }
    return x;
}

qreal TimelineHelper::visible_range_size_per_frame() const
{
    const int handle_width = alive::theme::TimelineSize::instance()->handle_width();
    const qreal effective_width = width() - 2 * handle_width;
    return effective_width / (m_timeline->comp_end() - m_timeline->comp_start());
}

qreal TimelineHelper::play_range_size_per_frame() const
{
    const int handle_width = alive::theme::TimelineSize::instance()->handle_width();
    const qreal effective_width = width() - 2 * handle_width;
    return effective_width / (m_timeline->range_end() - m_timeline->range_start());
}
} // namespace alive::gui
