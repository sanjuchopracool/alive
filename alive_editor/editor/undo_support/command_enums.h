#ifndef COMMAND_ENUMS_H
#define COMMAND_ENUMS_H

namespace alive {
enum class CommandType {
    e_None,
    e_FunctorCommand,
    e_PropertySetValue,
    e_PropertySetExpression,
    e_PropertyDisableExpression,
    e_AddRemoveNewLayer,
    e_LayerSetBool,
    e_LayerSetInt,
    e_LayerDragRange,
    e_LayerSetRange,
    e_LayersMove,
    e_KeyframeRemoveAll,
    e_KeyframeAdd,
    e_KeyframeRemove,
    e_KeyFramesMove,
    e_KeyFramesSetEasing,
    e_KeyFrameDisableExpression,
    e_CompositionSetting,
    e_LayerSetting,
    e_ShapeAddRemoveShape,
    e_ShapesMove,
    e_ObjectSetBool,
    e_SetPathShapeDirection,
    e_TimelineSetWorkArea,
    e_TimelineSelectFrame
};
}
#endif // COMMAND_ENUMS_H
