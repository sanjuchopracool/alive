#ifndef KEYFRAMEITEM_H
#define KEYFRAMEITEM_H

#include <map>
#include <core/model/property/update_listener.h>
#include <QPainterPath>
#include <QQuickPaintedItem>

namespace alive {
class TreeItem;
}
namespace alive::model {
class CompositionDocument;
class Property;
class KeyFrame;
} // namespace alive::model

namespace alive::gui {
class TimelineHelper;

struct KeyFrameStatus
{
    bool selected = false;
};

struct CompareKeyframes
{
    bool operator()(const model::KeyFrame *v1, const model::KeyFrame *v2) const;
};

class KeyFrameItem : public QQuickPaintedItem, public model::SimpleObserver
{
    Q_OBJECT
public:
    KeyFrameItem(QQuickItem *parent = nullptr);
    ~KeyFrameItem();

    void paint(QPainter *painter) override;
    Q_INVOKABLE void set_item(TreeItem *item);
    Q_INVOKABLE void set_timeline_helper(TimelineHelper *helper);

    void on_update(model::ISubject *) override;
signals:
    void clear_all_keyframes_from_selection();
    void add_keyframe_to_selection(model::KeyFrame *);
    void add_only_one_keyframe_to_selection(model::KeyFrame *);
    void remove_keyframe_from_selection(model::KeyFrame *);
    void clear_keyframes_from_selection(QList<model::KeyFrame *>);
    void edit_easing_between(QVariant);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void slot_clear_selection();

private:
    void reset_keyframes();
    bool select_one_keyframe(QPointF pos);
    bool clear_selection();
    bool select_one_keyframe(model::KeyFrame *);
    bool toggle_keyframe_selection(QPointF pos);
    model::KeyFrame *find_keyframe(QPointF pos);
    std::pair<model::KeyFrame *, model::KeyFrame *> find_keyframes_around(QPointF pos);
    QList<model::KeyFrame *> selected_frames() const;
    void clear_bounds();

private:
    model::Property *m_property = nullptr;
    TimelineHelper *m_helper = nullptr;

    std::map<model::KeyFrame *, KeyFrameStatus, CompareKeyframes> m_keyframes;

    bool m_sent_clear_selection = false;
    bool m_dragging = false;
    qreal m_drag_start_pos_x;
    std::pair<model::KeyFrame *, model::KeyFrame *> m_bounds;
};
} // namespace alive::gui
#endif // KEYFRAMEITEM_H
