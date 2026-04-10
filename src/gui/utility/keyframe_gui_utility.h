#ifndef KEYFRAME_GUI_UTILITY_H
#define KEYFRAME_GUI_UTILITY_H

#include <core/model/keyframes/keyframe.h>
#include <QPainterPath>

namespace alive::keyFrameGuiUtility {
const QPainterPath &path_for_keyframe(const alive::model::KeyFrame *keyframe);
void generate_keyframe_path();
} // namespace alive::keyFrameGuiUtility

#endif // KEYFRAME_GUI_UTILITY_H
