#ifndef EDITORIMAGELAYER_H
#define EDITORIMAGELAYER_H

#include <core/alive_types/common_types.h>

namespace alive::model {
struct FileBasedLayerSetting
{
public:
    bool operator!=(const FileBasedLayerSetting &other) const
    {
        return name != other.name || path != other.path;
    }

public:
    std::string name;
    std::string path;
};

} // namespace alive::model

#endif // EDITORIMAGELAYER_H
