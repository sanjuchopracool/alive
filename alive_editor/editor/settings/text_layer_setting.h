#ifndef EDITORTEXTLAYER_H
#define EDITORTEXTLAYER_H

#include <core/alive_types/common_types.h>

namespace alive::model {
struct TextLayerSetting
{
public:
    bool operator==(const TextLayerSetting &other) const
    {
        return (name == other.name) && (text == other.text);
    }
public:
    std::string name;
    std::string text;
};
} // namespace alive::model

#endif // EDITORTEXTLAYER_H
