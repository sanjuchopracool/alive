#include <editor/document/document.h>
#include <editor/undo_support/document_undo_manager.h>
#include <editor/undo_support/keyframe_commands.h>

namespace alive {
KeyframeCommand::KeyframeCommand(model::Property *property, CommandType type)
    : Command(type, {})
    , m_property(property)
{}

void KeyframeCommand::update_document()
{
    if (m_document) {
        m_document->undo_manager()->updated_item(m_property, m_roles);
    }
}

RemoveAllKeyFramesCommand::RemoveAllKeyFramesCommand(model::Property *property)
    : KeyframeCommand(property, CommandType::e_KeyframeRemoveAll)
{}

void RemoveAllKeyFramesCommand::redo()
{
    m_old_frames = m_property->remove_all_keyframes();
    update_document();
}

void RemoveAllKeyFramesCommand::undo()
{
    m_property->set_keyframes(m_old_frames);
    m_old_frames.clear();
    update_document();
}

AddKeyFrameCommand::AddKeyFrameCommand(KeyFrameTime t, model::Property *property)
    : KeyframeCommand(property, CommandType::e_KeyframeAdd)
    , m_t(t)
{}

void AddKeyFrameCommand::redo()
{
    m_keyframe = m_property->add_keyframe(m_t);
    update_document();
}

void AddKeyFrameCommand::undo()
{
    if (m_keyframe) {
        m_property->remove_keyframe(m_keyframe);
        m_keyframe = nullptr;
        update_document();
    }
}

RemoveKeyFrameCommand::RemoveKeyFrameCommand(KeyFrameTime t, model::Property *property)
    : KeyframeCommand(property, CommandType::e_KeyframeRemove)
    , m_t(t)
{}

void RemoveKeyFrameCommand::redo()
{
    m_keyframe = m_property->remove_keyframe(m_t);
    update_document();
}

void RemoveKeyFrameCommand::undo()
{
    if (m_keyframe) {
        m_property->add_keyframe(m_keyframe);
        m_keyframe = nullptr;
        update_document();
    }
}

MoveKeyFramesCommand::MoveKeyFramesCommand(const std::set<model::KeyFrame *> &keyframes,
                                           KeyFrameTime t)
    : Command(CommandType::e_KeyFramesMove, {})
    , m_keyframes(keyframes)
    , m_time(t)
{
}

void MoveKeyFramesCommand::redo()
{
    move_by(m_time);
}

void MoveKeyFramesCommand::undo()
{
    move_by(-m_time);
}

bool MoveKeyFramesCommand::merge_with(const Command *other)
{
    const MoveKeyFramesCommand *other_command = static_cast<const MoveKeyFramesCommand *>(other);
    if (m_keyframes == other_command->keyframes()) {
        m_time += other_command->time();
        return true;
    }

    return false;
}

void MoveKeyFramesCommand::move_by(KeyFrameTime t)
{
    using namespace model;
    std::set<Property *> properties;
    for (KeyFrame *keyframe : m_keyframes) {
        keyframe->set_time(keyframe->time() + t);
        properties.emplace(keyframe->property());
    }
    for (Property *property : properties) {
        property->sort_keyframes_by_time();
        if (m_document) {
            m_document->undo_manager()->updated_item(property, m_roles);
        }
    }
}

KeyFrameEasingCommand::KeyFrameEasingCommand(const model::EasingBetweenTwoFramesData &data)
    : Command(CommandType::e_KeyFramesSetEasing, {})
    , m_data(data)
{
    m_from_old = m_data.m_left_keyframe->out_tangent();
    m_to_old = m_data.m_right_keyframe->in_tangent();
}

void KeyFrameEasingCommand::redo()
{
    Vec2D new_from{m_data.m_ct1.x(), m_data.m_ct1.y()};
    Vec2D new_to{m_data.m_ct2.x(), m_data.m_ct2.y()};
    if (m_from_old != new_from || m_to_old != new_to) {
        m_data.m_left_keyframe->set_out_tangent(new_from);
        m_data.m_right_keyframe->set_in_tangent(new_to);
        m_data.m_left_keyframe->property()->notify_observers();
    }
}

void KeyFrameEasingCommand::undo()
{
    m_data.m_left_keyframe->set_out_tangent(m_from_old);
    m_data.m_right_keyframe->set_in_tangent(m_to_old);
    m_data.m_left_keyframe->property()->notify_observers();
}

bool KeyFrameEasingCommand::merge_with(const Command *other)
{
    const KeyFrameEasingCommand *other_command = static_cast<const KeyFrameEasingCommand *>(other);
    if (m_data.m_left_keyframe->property() == other_command->m_data.m_left_keyframe->property()
        && m_data.m_left_keyframe == other_command->m_data.m_left_keyframe
        && m_data.m_right_keyframe == other_command->m_data.m_right_keyframe) {
        m_data = other_command->m_data;
        return true;
    }

    return false;
}

} // namespace alive
