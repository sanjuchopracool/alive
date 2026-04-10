#ifndef TIMELINEMODELHELPERS_H
#define TIMELINEMODELHELPERS_H

#include <core/alive_types/common_types.h>
#include <core/model/property/property.h>
#include <QObject>

namespace alive::model {
struct LayerInOutPoints
{
    Q_GADGET
    Q_PROPERTY(FrameTimeType in_point MEMBER in_point CONSTANT)
    Q_PROPERTY(FrameTimeType out_point MEMBER out_point CONSTANT)
public:
    KeyFrameTime in_point = 0;
    KeyFrameTime out_point = 0;
};

struct KeyFrameStatusValue : public model::KeyFrameStatus
{
    Q_GADGET
    Q_PROPERTY(bool at_keyframe MEMBER at_keyframe CONSTANT)
    Q_PROPERTY(bool on_left MEMBER on_left CONSTANT)
    Q_PROPERTY(bool on_right MEMBER on_right CONSTANT)
    Q_PROPERTY(bool scripted MEMBER scripted CONSTANT)
    Q_PROPERTY(bool leftScripted MEMBER left_scripted CONSTANT)
    Q_PROPERTY(bool rightScripted MEMBER right_scripted CONSTANT)
    Q_PROPERTY(KeyFrameTime previous MEMBER previous CONSTANT)
    Q_PROPERTY(KeyFrameTime next MEMBER next CONSTANT)
    Q_PROPERTY(KeyFrameTime current MEMBER current CONSTANT)
public:
    KeyFrameStatusValue() = default;
    KeyFrameStatusValue(const model::KeyFrameStatus &base_data);
};

struct DragSupportStatusValue
{
    Q_GADGET
    Q_PROPERTY(bool support_drag MEMBER m_support_drag CONSTANT)
    Q_PROPERTY(bool support_drop MEMBER m_support_drop CONSTANT)
public:
    bool m_support_drag = false;
    bool m_support_drop = false;
};

} // namespace alive::model

Q_DECLARE_METATYPE(alive::model::LayerInOutPoints)
Q_DECLARE_METATYPE(alive::model::KeyFrameStatusValue)
Q_DECLARE_METATYPE(alive::model::DragSupportStatusValue)

#endif // TIMELINEMODELHELPERS_H
