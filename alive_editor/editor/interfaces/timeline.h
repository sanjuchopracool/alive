#ifndef I_TIMELINE_H
#define I_TIMELINE_H

namespace alive
{

class ITimeline
{
public:
    virtual ~ITimeline() {}
    virtual int play_start() const = 0;
    virtual int play_end() const = 0;
    virtual void set_play_range(int play_start, int play_end) = 0;
    virtual void set_range(int range_start, int range_end) = 0;

    virtual int range_start() const = 0;
    virtual int range_end() const = 0;
};

}
#endif // I_TIMELINE_H
