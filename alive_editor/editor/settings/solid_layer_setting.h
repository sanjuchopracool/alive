#ifndef EDITORSOLIDLAYER_H
#define EDITORSOLIDLAYER_H

#include <core/alive_types/common_types.h>

namespace alive::model {
struct SolidLayerSetting
{

public:
    bool operator==(const SolidLayerSetting &other) const
    {
        return (width == other.width) && (height == other.height) && (name == other.name)
               && (color == other.color);
    }

public:
    int width = 0;
    int height = 0;
    std::string name;
    Color4ub color;
};
} // namespace alive::model

#endif // EDITORSOLIDLAYER_H
