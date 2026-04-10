#ifndef UNDOSTACK_H
#define UNDOSTACK_H

#include <Corrade/Containers/Pointer.h>
#include <core/alive_types/class_helpers.h>
#include <core/alive_types/common_types.h>
#include <vector>

namespace alive {

class Command;
class UndoStack
{
    NON_COPYABLE(UndoStack)
public:
    UndoStack();
    ~UndoStack();

    bool push_command(alive_ptr<Command> cmd);

    bool redo();
    bool undo();

private:
    void update_commands_size();
    void check_size_constraints();

private:
    std::vector<alive_ptr<Command>> m_commands;
    size_t m_current_index = 0;
};

} // namespace alive

#endif // UNDOSTACK_H
