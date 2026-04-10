#ifndef KEYFRAMECOMMANDS_H
#define KEYFRAMECOMMANDS_H

#include <editor/undo_support/command.h>
#include <editor/undo_support/command_enums.h>
#include <core/model/property/property.h>
#include <set>
#include <core/model/property/easing.h>

namespace alive::model {
class KeyFrame;
}
namespace alive {

class KeyframeCommand : public Command
{
public:
    KeyframeCommand(model::Property *property, CommandType type);

    const model::Property *property() const { return m_property; }

protected:
    void update_document();
    model::Property *m_property;
};

class RemoveAllKeyFramesCommand : public KeyframeCommand
{
public:
    RemoveAllKeyFramesCommand(model::Property *property);
    void redo() override;
    void undo() override;

private:
    model::KeyFrames m_old_frames;
};

class AddKeyFrameCommand : public KeyframeCommand
{
public:
    AddKeyFrameCommand(KeyFrameTime t, model::Property *property);
    void redo() override;
    void undo() override;

private:
    KeyFrameTime m_t;
    model::KeyFrame *m_keyframe = nullptr;
};

class RemoveKeyFrameCommand : public KeyframeCommand
{
public:
    RemoveKeyFrameCommand(KeyFrameTime t, model::Property *property);
    void redo() override;
    void undo() override;

private:
    KeyFrameTime m_t;
    model::KeyFrame *m_keyframe = nullptr;
};

class MoveKeyFramesCommand : public Command
{
public:
    MoveKeyFramesCommand(const std::set<model::KeyFrame *> &keyframes, KeyFrameTime t);
    void redo() override;
    void undo() override;

    bool merge_with(const Command *other) override;
    const std::set<model::KeyFrame *> &keyframes() const { return m_keyframes; }
    KeyFrameTime time() const { return m_time; }
private:
    void move_by(KeyFrameTime t);

private:
    std::set<model::KeyFrame *> m_keyframes;
    KeyFrameTime m_time;
};

class KeyFrameEasingCommand : public Command
{
public:
    KeyFrameEasingCommand(const model::EasingBetweenTwoFramesData &data);

    void redo() override;
    void undo() override;
    bool merge_with(const Command *other) override;
    Vec2D m_from_old;
    Vec2D m_to_old;
    model::EasingBetweenTwoFramesData m_data;
};

class KeyFrameDisableExpressionCommand : public KeyframeCommand
{
public:
    KeyFrameDisableExpressionCommand(model::KeyFrame *keyframe,
                                     bool disable)
        : KeyframeCommand(keyframe->property(), CommandType::e_KeyFrameDisableExpression)
        , m_keyframe(keyframe)
        , m_disable(disable)
    {
    }

    void redo() override
    {
        m_keyframe->enable_expression(!m_disable);
        m_keyframe->property()->mark_dirty();
        update_document();
    }

    void undo() override
    {
        m_keyframe->enable_expression(m_disable);
        m_keyframe->property()->mark_dirty();
        update_document();
    }


private:
    bool m_disable;
    model::KeyFrame* m_keyframe = nullptr;
};
} // namespace alive

#endif // KEYFRAMECOMMANDS_H
