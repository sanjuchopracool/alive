#include "timeline.h"

#include <gui/document/composition_document.h>
#include <core/model/composition.h>
#include <QDebug>
#include <QVersionNumber>

namespace alive::gui {
namespace {
QVersionNumber s_version(1, 0, 0);

float k_precision_factor = 1000;
int frame_type_to_msec(FrameTimeType frame, FrameTimeType framerate)
{
    return static_cast<int>(frame * 1000 * k_precision_factor / framerate);
}

} // namespace

TimeLine::TimeLine(const model::CompositionDocument *doc, QObject *parent)
    : QObject(parent)
    , m_document(doc)
{
    m_timeline = new QTimeLine(1000, this);
    m_timeline->setEasingCurve(QEasingCurve::Linear);
    m_timeline->setLoopCount(0);

    connect(m_timeline, &QTimeLine::frameChanged, this, &TimeLine::play_frame_changed);
    update_comp_settings();
    connect(doc,
            &model::CompositionDocument::composition_settings_updated,
            this,
            &TimeLine::update_comp_settings);
}

TimeLine::~TimeLine() {}

int TimeLine::play_duration() const
{
    return m_timeline->duration();
}

void TimeLine::set_play_range(int play_start, int play_end)
{
    if ((play_end > play_start) && is_valid_range_value(play_start)
        && is_valid_range_value(play_end)) {
        if ((play_start != this->play_start()) || (play_end != this->play_end())) {
            m_timeline->setDuration((play_end - play_start) * m_msec_per_frame);
            m_timeline->setFrameRange(play_start, play_end);
            emit play_range_changed();
        }
    }
}

int TimeLine::play_start() const
{
    return m_timeline->startFrame();
}

int TimeLine::play_end() const
{
    return m_timeline->endFrame();
}

int TimeLine::play_frame() const
{
    return m_timeline->currentFrame();
}

void TimeLine::set_range(int range_start, int range_end)
{
    if (is_valid_range_value(range_start) && is_valid_range_value(range_end)) {
        if (m_end_frame != range_end || m_start_frame != range_start) {
            m_start_frame = range_start;
            m_end_frame = range_end;

            // update scale also
            //            m_current_scale = composition_duration() / range_duration();
            //            emit scale_changed(m_current_scale);
            emit range_changed();
        }
    }
}

int TimeLine::range_duration() const
{
    return (m_end_frame - m_start_frame) * m_msec_per_frame;
}

void TimeLine::set_range_start(int range_start)
{
    if (m_comp_start_frame != range_start && is_valid_range_value(range_start)) {
        m_comp_start_frame = range_start;
        emit range_changed();
    }
}

int TimeLine::range_start() const
{
    return m_start_frame;
}

int TimeLine::fps() const
{
    return m_fps;
}

void TimeLine::set_range_end(int range_end)
{
    if (m_comp_end_frame != range_end && is_valid_range_value(range_end)) {
        m_comp_end_frame = range_end;
        emit range_changed();
    }
}

int TimeLine::range_end() const
{
    return m_end_frame;
}

int TimeLine::comp_start() const
{
    return m_comp_start_frame;
}

int TimeLine::comp_end() const
{
    return m_comp_end_frame;
}

int TimeLine::composition_duration_in_msec() const
{
    return m_comp_duration;
}

int TimeLine::msec_per_frame() const
{
    return m_msec_per_frame;
}

void TimeLine::select_frame(int frame)
{
    // if (frame >= m_start_frame && frame <= m_end_frame) {
    if (m_current_frame != frame) {
        m_current_frame = frame;
        emit selected_frame_changed(m_current_frame);
        }
        // }
}

double TimeLine::current_scale() const
{
    return m_current_scale;
}

int TimeLine::current_frame() const
{
    return m_current_frame;
}

int TimeLine::duration() const
{
    return m_timeline->duration();
}

QTimeLine::State TimeLine::state() const
{
    return m_timeline->state();
}

bool TimeLine::is_running() const
{
    return (m_timeline->state() == QTimeLine::Running);
}

void TimeLine::start()
{
    m_timeline->start();
    emit run_state_changed();
}

void TimeLine::resume()
{
    m_timeline->resume();
    emit run_state_changed();
}

void TimeLine::stop()
{
    m_timeline->stop();
    emit run_state_changed();
    emit stopped();
    select_frame(m_timeline->currentFrame());
}

void TimeLine::scale_range(double scale)
{
    if (scale < 1.0)
        scale = 1.0;

    const int current_range_duration = range_duration();
    double new_range_duration = current_range_duration / scale;
    if (new_range_duration > msec_per_frame()) {
        int current_frame = this->current_frame();

        if (current_frame < 0)
            current_frame = 0;

        if (current_frame > m_comp_duration)
            current_frame = m_comp_duration;

        double current_start_fraction = static_cast<double>(current_frame - m_comp_start_frame)
                                        / (m_comp_end_frame - m_comp_start_frame);
        int new_start = current_frame - (current_start_fraction * new_range_duration);
        if (new_start < 0)
            new_start = 0;

        int new_end = new_start + new_range_duration;
        if (new_end > current_range_duration) {
            new_end = current_range_duration;
            new_start = new_end - new_range_duration;
        }

        set_range(new_start, new_start + new_range_duration);
    }
}

bool TimeLine::is_valid_range_value(int value)
{
    return (value >= m_comp_start_frame) && (value <= m_comp_end_frame);
}

void TimeLine::update_comp_settings()
{
    const auto comp = m_document->composition();
    m_msec_per_frame = 1000 / comp->framerate();
    m_comp_duration = (comp->out_point() - comp->in_point()) * m_msec_per_frame;
    m_comp_start_frame = comp->in_point();
    m_comp_end_frame = comp->out_point();

    // visible range, play area
    m_start_frame = m_comp_start_frame;
    m_end_frame = m_comp_end_frame;
    m_fps = comp->framerate();

    m_timeline->setDuration(m_comp_duration);
    m_timeline->setFrameRange(m_comp_start_frame, m_comp_end_frame);
    m_timeline->setUpdateInterval(m_msec_per_frame);

    emit range_changed();
    emit fps_changed();
    emit selected_frame_changed(m_current_frame);
    emit play_range_changed();
}
} // namespace alive::gui
