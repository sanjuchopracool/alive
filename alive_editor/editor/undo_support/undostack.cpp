#include <editor/undo_support/command.h>
#include <editor/undo_support/undostack.h>

namespace alive {

UndoStack::UndoStack() {}

UndoStack::~UndoStack() {}

bool UndoStack::push_command(alive_ptr<Command> cmd)
{
    bool result = false;
    Command *cmd_to_execute = cmd.get();
    if (m_commands.empty() || (m_current_index == 0)) {
        m_commands.clear();
        m_commands.push_back(std::move(cmd));
        ++m_current_index;
        result = true;
    } else {
        auto &cmd_at_top = m_commands[m_current_index - 1];
        if (cmd_at_top->can_merge(cmd_to_execute) && cmd_at_top->merge_with(cmd_to_execute)) {
            result = false;
            update_commands_size();
        } else if (m_current_index == m_commands.size()) {
            // It's new non mergable command, pushback or update
            m_commands.push_back(std::move(cmd));
            ++m_current_index;
            result = true;
        } else {
            m_commands[m_current_index] = std::move(cmd);
            ++m_current_index;
            update_commands_size();
        }
    }
    cmd_to_execute->redo();
    return result;
}

bool UndoStack::redo()
{
    check_size_constraints();
    if (m_current_index < m_commands.size()) {
        m_commands[m_current_index]->redo();
        ++m_current_index;
        return true;
    }

    return false;
}

bool UndoStack::undo()
{
    check_size_constraints();
    if (m_current_index > 0) {
        --m_current_index;
        m_commands[m_current_index]->undo();
    }
    return false;
}

void UndoStack::update_commands_size()
{
    m_commands.erase(m_commands.begin() + m_current_index, m_commands.end());
}

void UndoStack::check_size_constraints()
{
    assertm(m_current_index <= m_commands.size() && m_current_index >= 0,
            "commands count should always be greater than 0 and less than or equal to stack size");
}

} // namespace alive
