#ifndef TIMELINE_COMMANDS_H
#define TIMELINE_COMMANDS_H

#include <editor/undo_support/command.h>
#include <editor/undo_support/command_enums.h>

namespace alive {
class ITimeline;
}

namespace alive {
class TimelineCommands : public Command
{
public:
    TimelineCommands(ITimeline *timeline,
                     CommandType type,
                     std::string name)
        : Command(type, std::move(name))
        , m_timeline(timeline)
    {}
    const ITimeline *timeline() const { return m_timeline; }

protected:
    ITimeline *m_timeline;
};

class TimelineRangeCommand : public TimelineCommands
{
public:
    enum RangeType { e_Visible, e_WorkArea };

    TimelineRangeCommand(ITimeline *timeline,
                         RangeType type,
                         int new_start,
                         int new_end,
                         bool merge = false,
                         std::string name = "")
        : TimelineCommands(timeline, CommandType::e_TimelineSetWorkArea, std::move(name))
        , m_range_type(type)
        , m_new_start(new_start)
        , m_new_end(new_end)
        , m_merge(merge)
    {}

    bool merge_with(const Command *other) override;

    void redo() override;
    void undo() override;

private:
    RangeType m_range_type;
    int m_new_start;
    int m_new_end;
    bool m_merge = false;

    int m_prev_start;
    int m_prev_end;
};

} // namespace alive::commands

#endif // TIMELINE_COMMANDS_H
