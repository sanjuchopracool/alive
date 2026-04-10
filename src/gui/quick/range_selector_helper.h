#ifndef RANGESELECTORHELPER_H
#define RANGESELECTORHELPER_H

#include <gui/composition/timeline.h>
#include <set>
#include <QObject>

namespace alive::model {
class KeyFrame;
class CompositionDocument;
}
namespace alive::gui {
class TimeLine;
class TimelineHelper : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int current_frame READ current_frame NOTIFY current_frame_changed)
    Q_PROPERTY(int play_frame READ play_frame NOTIFY play_frame_changed)
    Q_PROPERTY(int is_running READ is_running NOTIFY runstate_changed)
    Q_PROPERTY(int range_start READ range_start NOTIFY range_changed)
    Q_PROPERTY(int range_end READ range_end NOTIFY range_changed)
    Q_PROPERTY(qreal fps READ fps NOTIFY fps_changed)
    Q_PROPERTY(qreal width READ width WRITE set_width NOTIFY width_changed)
    Q_PROPERTY(qreal width_per_frame MEMBER m_width_per_frame NOTIFY width_changed)

    Q_PROPERTY(qreal selected_frame_position MEMBER m_selected_frame_position NOTIFY
                   selected_frame_position_changed)
    Q_PROPERTY(qreal selected_frame_range_position MEMBER m_selected_frame_range_position NOTIFY
                   selected_frame_range_position_changed)
    Q_PROPERTY(qreal m_visible_range_start_x MEMBER m_visible_range_start_x NOTIFY
                   visible_range_start_x_changed)
    Q_PROPERTY(
        qreal visible_range_width MEMBER m_visible_range_width NOTIFY visible_range_width_changed)
    Q_PROPERTY(qreal m_visible_play_range_start_x MEMBER m_visible_play_range_start_x NOTIFY
                   visible_play_range_start_x_changed)
    Q_PROPERTY(qreal visible_play_range_width MEMBER m_visible_play_range_width NOTIFY
                   visible_play_range_width_changed)

    Q_PROPERTY(
        qreal m_play_range_start_x MEMBER m_play_range_start_x NOTIFY play_range_start_x_changed)
    Q_PROPERTY(qreal play_range_width MEMBER m_play_range_width NOTIFY play_range_width_changed)
public:
    TimelineHelper(TimeLine *timeline, QObject *parent = nullptr);

    Q_INVOKABLE void select_frame_for_x(qreal x) const;
    Q_INVOKABLE void select_range_start_for_x(qreal x);
    Q_INVOKABLE void select_range_end_for_x(qreal x);
    Q_INVOKABLE void visible_range_move_started(qreal x);
    Q_INVOKABLE void visible_range_moved(qreal x);

    Q_INVOKABLE void select_play_start_for_x(qreal x);
    Q_INVOKABLE void select_play_end_for_x(qreal x);
    Q_INVOKABLE void play_range_move_started(qreal x);
    Q_INVOKABLE void play_range_moved(qreal x);
    Q_INVOKABLE qreal position_for_frame(int frame) const;
    Q_INVOKABLE void select_current_frame(KeyFrameTime frame) const;
    Q_INVOKABLE int frame_for_click(qreal x) const;

    qreal visible_range_start_x() const { return m_visible_range_start_x; }
    qreal visible_range_width() const { return m_visible_range_width; }
    qreal visible_play_range_start_x() const { return m_visible_play_range_start_x; }
    qreal visible_play_range_width() const { return m_visible_play_range_width; }

    int current_frame() const;
    int play_frame() const;
    bool is_running() const;

    int range_start() const;
    int range_end() const;
    qreal fps() const;

    qreal width() const { return m_width; }
    void set_width(qreal width);
    qreal width_per_frame() const { return m_width_per_frame; }
    const model::CompositionDocument *document() const;

    TimeLine *timeline() const { return m_timeline; }

    qreal selected_frame_position_x() const { return m_selected_frame_position; }

public slots:
    void slot_clear_all_keyframes_from_selection();
    void slot_add_keyframe_to_selection(model::KeyFrame *keyframe);
    void slot_remove_keyframe_from_selection(model::KeyFrame *keyframe);
    void slot_add_only_one_keyframe_to_selection(model::KeyFrame *keyframe);
    void slot_frames_moved(KeyFrameTime frames);
signals:
    void selected_frame_position_changed();
    void selected_frame_range_position_changed();
    void visible_range_width_changed();
    void visible_range_start_x_changed();
    void play_range_width_changed();
    void play_range_start_x_changed();
    void visible_play_range_start_x_changed();
    void visible_play_range_width_changed();
    void current_frame_changed(int);
    void play_frame_changed(int);
    void runstate_changed();
    void range_changed();
    void fps_changed();
    void width_changed();
    void clear_all_keyframes_from_selection();

private:
    int frame_for_range_click(qreal x) const;
    qreal range_position_for_frame(int frame) const;
    qreal visible_range_size_per_frame() const;
    qreal play_range_size_per_frame() const;

private:
    TimeLine *m_timeline = nullptr;
    qreal m_width = 1;
    qreal m_selected_frame_position;
    qreal m_selected_frame_range_position;
    qreal m_visible_range_start_x;
    qreal m_visible_range_width;

    qreal m_visible_play_range_start_x;
    qreal m_visible_play_range_width;

    qreal m_play_range_start_x;
    qreal m_play_range_width;

    qreal m_visible_range_move_start;
    qreal m_play_range_move_start;

    //
    qreal m_width_per_frame = 0;
    std::set<model::KeyFrame *> m_selected_keyframes;
};
} // namespace alive::gui

#endif // RANGESELECTORHELPER_H
