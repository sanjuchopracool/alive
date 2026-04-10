#ifndef TIMELINE_H
#define TIMELINE_H

#include <core/alive_types/common_types.h>
#include <editor/interfaces/timeline.h>
#include <QTimeLine>

namespace alive::model {
class CompositionDocument;
}

namespace alive::gui {

class TimeLineData;
class TimeLine : public QObject, public ITimeline
{
    Q_OBJECT
public:
    TimeLine(const model::CompositionDocument *doc = nullptr, QObject *parent = nullptr);
    ~TimeLine();

    int play_duration() const;
    void set_play_range(int play_start, int play_end) override;

    int play_start() const override;
    int play_end() const override;
    int play_frame() const;

    void set_range(int range_start, int range_end) override;
    int range_duration() const;

    void set_range_start(int range_start);
    int range_start() const override;

    int fps() const;
    void set_range_end(int range_end);
    int range_end() const override;

    int comp_start() const;
    int comp_end() const;
    int composition_duration_in_msec() const;

    int msec_per_frame() const;

    void select_frame(int frame);
    int current_frame() const;

    double current_scale() const;

    int duration() const;
    QTimeLine::State state() const;
    bool is_running() const;

    const model::CompositionDocument *document() const { return m_document; }

public slots:
    void start();
    void resume();
    void stop();
    void scale_range(double scale);
signals:
    void selected_frame_changed(int);
    void play_frame_changed(int);
    void range_changed();
    void play_range_changed();
    void scale_changed(double);
    void stopped();
    void run_state_changed();
    void fps_changed();

private:
    bool is_valid_range_value(int value);

private slots:
    void update_comp_settings();

private:
    const model::CompositionDocument *m_document = nullptr;
    QTimeLine *m_timeline = nullptr;

    int m_comp_duration = 1000;
    int m_comp_start_frame = 0;
    int m_comp_end_frame = 30;
    int m_msec_per_frame = 33;

    FrameTimeType m_current_frame = 0.0f;

    // visible range
    FrameTimeType m_start_frame = 0.0f;
    FrameTimeType m_end_frame = 30.0f;
    FrameTimeType m_fps;

    float m_current_scale = 1;
};
} // namespace alive::gui
#endif // TIMELINE_H
