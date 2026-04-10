#include "timeline_settings.h"

namespace alive::settings {
TimelineSettings::TimelineSettings()
{
    
}

int TimelineSettings::minimum_visible_range()
{
    return 10;
}

int TimelineSettings::minimum_play_range()
{
    return 5;
}
} // namespace alive::settings
