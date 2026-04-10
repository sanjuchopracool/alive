#include <editor/undo_support/timeline_commands.h>
#include <editor/interfaces/timeline.h>
#include <core/logging/logger.h>
namespace alive {

bool TimelineRangeCommand::merge_with(const Command *other)
{
    if (!m_merge) {
        return false;
    }

    const TimelineRangeCommand *other_command = static_cast<const TimelineRangeCommand *>(other);
    if (other_command->m_merge && m_range_type == other_command->m_range_type) {
        m_new_start = other_command->m_new_start;
        m_new_end = other_command->m_new_end;
        return true;
    }
    return false;
}
void TimelineRangeCommand::redo()
{
    if (m_range_type == e_WorkArea) {
        m_prev_start = m_timeline->play_start();
        m_prev_end = m_timeline->play_end();
    } else {
        m_prev_start = m_timeline->range_start();
        m_prev_end = m_timeline->range_end();
    }

    if (m_new_start > m_new_end) {
        std::swap(m_new_start, m_new_end);
    }

    if (m_range_type == e_WorkArea) {
        m_timeline->set_play_range(m_new_start, m_new_end);
    } else {
        m_timeline->set_range(m_new_start, m_new_end);
    }
}

void TimelineRangeCommand::undo()
{
    if (m_range_type == e_WorkArea) {
        m_timeline->set_play_range(m_prev_start, m_prev_end);
    } else {
        m_timeline->set_range(m_prev_start, m_prev_end);
    }
}

} // namespace alive::commands
