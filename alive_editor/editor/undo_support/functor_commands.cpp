#include <editor/undo_support/command_enums.h>
#include <editor/undo_support/functor_commands.h>

namespace alive {

FunctorCommand::FunctorCommand(const FunctionType &do_func,
                               const FunctionType &undo_func,
                               const FunctionType &del_func,
                               std::string name)
    : Command(CommandType::e_FunctorCommand, name)
    , m_do_func(do_func)
    , m_undo_func(undo_func)
    , m_del_func(del_func)
{}

FunctorCommand::~FunctorCommand()
{
    if (m_del_func) {
        m_del_func(this);
    }
}

void FunctorCommand::redo()
{
    if (m_do_func) {
        m_do_func(this);
        m_done = true;
    }
}

void FunctorCommand::undo()
{
    if (m_undo_func) {
        m_undo_func(this);
        m_done = false;
    }
}

alive_ptr<Command> FunctorCommand::create_functor_command(const FunctionType &do_func,
                                                                 const FunctionType &undo_func,
                                                                 const FunctionType &del_func,
                                                                 std::string name)
{
    return Corrade::Containers::pointer<FunctorCommand>(do_func, undo_func, del_func, name);
}

void FunctorCommand::set_int(int value) {
    m_data = value;
}

int FunctorCommand::get_int()
{
    return std::get<int>(m_data);
}

void FunctorCommand::set_property_data(PropertyData data) {
    m_data = data;
}

const FunctorCommand::PropertyData* FunctorCommand::get_property_data() const {
    return std::get_if<PropertyData>(&m_data);
}

} // namespace alive
