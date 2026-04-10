#ifndef KEYFRAME_SELECTION_MANAGER_H
#define KEYFRAME_SELECTION_MANAGER_H

#include <core/alive_types/common_types.h>
#include <QObject>
#include <QPointF>

namespace alive::model {
class KeyFrame;
class CompositionDocument;
class Property;
} // namespace alive::model
namespace alive::gui {
class TimelineHelper;
}

using namespace alive;
namespace alive {
struct KeyFrameSelectionManagerPrivateData;
class KeyFrameSelectionManager : public QObject
{
    Q_OBJECT
public:
    KeyFrameSelectionManager(alive::model::CompositionDocument *doc,
                             const alive::gui::TimelineHelper *helper,
                             QObject *parent = nullptr);
    ~KeyFrameSelectionManager();

    bool clicked_at(model::Property *property, const QPointF &position, bool open_editor);
    bool is_selected() const;
    model::KeyFrame *only_selected_frame() const;

public slots:
    void slot_clear_all_keyframes_from_selection();
    void slot_frames_moved(KeyFrameTime frames);

private slots:
    void slot_add_keyframe_to_selection(model::KeyFrame *keyframe);
    void slot_remove_keyframe_from_selection(model::KeyFrame *keyframe);
    void slot_add_only_one_keyframe_to_selection(model::KeyFrame *keyframe);
signals:
    void updated_selection();
    void show_bezier_editor(model::KeyFrame *left, model::KeyFrame *right);

private:
    std::unique_ptr<KeyFrameSelectionManagerPrivateData> m_d;
};
} // namespace alive

#endif // KEYFRAME_SELECTION_MANAGER_H
