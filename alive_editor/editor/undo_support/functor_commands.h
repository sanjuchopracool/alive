#ifndef FUNCTOR_COMMANDS_H
#define FUNCTOR_COMMANDS_H

#include <core/alive_types/common_types.h>
#include <editor/undo_support/command.h>
#include <functional>
#include <string>
#include <variant>

namespace alive {

namespace model {
class Property;
}

class FunctorCommand : public Command
{
public:
    struct PropertyData{
        model::Property* property = nullptr;
        int index= -1;
    };

    using Data = std::variant<int, PropertyData>;
    using FunctionType = std::function<void(FunctorCommand *)>;
    FunctorCommand(const FunctionType &do_func,
                   const FunctionType &undo_func,
                   const FunctionType &del_func,
                   std::string name = {});

    ~FunctorCommand();

    void redo() override;
    void undo() override;

    bool is_done() const { return m_done; }

    static alive_ptr<Command> create_functor_command(
        const FunctionType &do_func,
        const FunctionType &undo_func,
        const FunctionType &del_func = [](FunctorCommand *) {},
        std::string name = {});

    void set_int(int);
    int get_int();

    const PropertyData* get_property_data() const;
    void set_property_data(PropertyData data);
private:
    FunctionType m_do_func = nullptr;
    FunctionType m_undo_func = nullptr;
    FunctionType m_del_func = nullptr;
    bool m_done = false;
    Data m_data;
};

} // namespace alive

#endif // FUNCTOR_COMMANDS_H
