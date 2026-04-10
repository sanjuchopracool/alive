#ifndef TIMELINESETTINGS_H
#define TIMELINESETTINGS_H

namespace alive::settings {
class TimelineSettings
{
public:
    int minimum_visible_range();
    int minimum_play_range();

    static TimelineSettings &instance()
    {
        static TimelineSettings the_instance;
        return the_instance;
    }

private:
    TimelineSettings();
    TimelineSettings(const TimelineSettings &) = delete;
    TimelineSettings(TimelineSettings &&) = delete;
    TimelineSettings &operator=(const TimelineSettings &) = delete;
    TimelineSettings &operator=(TimelineSettings &&) = delete;
};
} // namespace alive::settings

#endif // TIMELINESETTINGS_H
