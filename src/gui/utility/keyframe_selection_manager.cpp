#include "keyframe_selection_manager.h"
#include <core/model/keyframes/keyframe.h>
#include <core/model/property/property.h>
#include <gui/document/composition_document.h>
#include <gui/quick/range_selector_helper.h>
#include <gui/utility/keyframe_gui_utility.h>
#include <set>

namespace alive {

struct KeyFrameSelectionManagerPrivateData
{
    KeyFrameSelectionManagerPrivateData(alive::model::CompositionDocument *t_doc,
                                        const alive::gui::TimelineHelper *t_helper)
        : doc(t_doc)
        , helper(t_helper)
    {}
    alive::model::CompositionDocument *doc = nullptr;
    const alive::gui::TimelineHelper *helper = nullptr;
    std::set<alive::model::KeyFrame *> selected_keyframes;
};

KeyFrameSelectionManager::KeyFrameSelectionManager(model::CompositionDocument *doc,
                                                   const gui::TimelineHelper *t_helper,
                                                   QObject *parent)
    : QObject(parent)
    , m_d(std::make_unique<KeyFrameSelectionManagerPrivateData>(doc, t_helper))
{}

KeyFrameSelectionManager::~KeyFrameSelectionManager() {}

bool KeyFrameSelectionManager::is_selected() const
{
    return !m_d->selected_keyframes.empty();
}

model::KeyFrame *KeyFrameSelectionManager::only_selected_frame() const
{
    return (m_d->selected_keyframes.size() == 1) ? *m_d->selected_keyframes.begin() : nullptr;
}

bool KeyFrameSelectionManager::clicked_at(model::Property *property,
                                          const QPointF &position,
                                          bool open_editor)
{
    model::KeyFrame *clicked_keyframe = nullptr;
    model::KeyFrame *prev_keyframe = nullptr;
    model::KeyFrame *next_keyframe = nullptr;
    for (auto *keyframe : property->get_keyframes()) {
        qreal frame_x = m_d->helper->position_for_frame(keyframe->time());
        QPointF pos_to_check(position.x() - frame_x, position.y());
        if (alive::keyFrameGuiUtility::path_for_keyframe(keyframe).contains(pos_to_check)) {
            clicked_keyframe = keyframe;
            break;
        }
        if (frame_x < position.x()) {
            prev_keyframe = keyframe;
        }

        if (frame_x > position.x()) {
            next_keyframe = keyframe;
            break;
        }
    }

    bool select = false;
    bool clear_all = false;

    if (clicked_keyframe) {
        bool selected = clicked_keyframe->is_selected_in_editor();
        if (open_editor) {
            select = !selected;
        } else {
            if (!selected) {
                select = true;
                clear_all = true;
            }
        }
    } else {
        // clear selection
        if (!open_editor) {
            clear_all = true;
        }
    }

    if (clear_all && select) {
        slot_add_only_one_keyframe_to_selection(clicked_keyframe);
        return true;
    } else if (clear_all) {
        slot_clear_all_keyframes_from_selection();
        return true;
    } else if (select) {
        slot_add_keyframe_to_selection(clicked_keyframe);
        return true;
    }

    if (open_editor && prev_keyframe && next_keyframe) {
        emit show_bezier_editor(prev_keyframe, next_keyframe);
        return true;
    }

    return false;
}

void KeyFrameSelectionManager::slot_clear_all_keyframes_from_selection()
{
    for (model::KeyFrame *m : m_d->selected_keyframes) {
        m->select_in_editor(false);
    };
    m_d->selected_keyframes.clear();
    emit updated_selection();
}

void KeyFrameSelectionManager::slot_add_keyframe_to_selection(model::KeyFrame *keyframe)
{
    keyframe->select_in_editor(true);
    m_d->selected_keyframes.emplace(keyframe);
    emit updated_selection();
}

void KeyFrameSelectionManager::slot_remove_keyframe_from_selection(model::KeyFrame *keyframe)
{
    keyframe->select_in_editor(false);
    m_d->selected_keyframes.erase(keyframe);
    emit updated_selection();
}

void KeyFrameSelectionManager::slot_add_only_one_keyframe_to_selection(model::KeyFrame *keyframe)
{
    for (model::KeyFrame *m : m_d->selected_keyframes) {
        m->select_in_editor(false);
    };
    m_d->selected_keyframes.clear();
    keyframe->select_in_editor(true);
    m_d->selected_keyframes.emplace(keyframe);
    emit updated_selection();
}

void KeyFrameSelectionManager::slot_frames_moved(KeyFrameTime frames)
{
    if (!m_d->selected_keyframes.empty()) {
        m_d->doc->move_keyframes(m_d->selected_keyframes, frames);
    }
}

} // namespace alive
