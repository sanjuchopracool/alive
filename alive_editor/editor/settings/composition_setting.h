#ifndef COMPOSITION_SETTING_H
#define COMPOSITION_SETTING_H

#include <core/alive_types/common_types.h>

namespace alive {
class Document;
struct CompositionSetting
{
public:
    bool operator==(const CompositionSetting &other) const
    {
        return (width == other.width) && (height == other.height) && (name == other.name)
        && (fps == other.fps) && (duration == other.duration) && (bg_color == other.bg_color)
            && (show_bg_color == other.show_bg_color);
    }

public:
    int width = 0;
    int height = 0;
    int duration = 2;
    FrameTimeType fps;
    std::string name;
    const Document *document = nullptr;
    Color4ub bg_color;
    bool show_bg_color;
};
} // namespace alive::model

#endif  //COMPOSITION_SETTING_H
