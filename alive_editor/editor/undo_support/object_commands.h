#ifndef OBJECT_COMMANDS_H
#define OBJECT_COMMANDS_H

#include <editor/undo_support/command.h>

namespace alive::model {
class Object;
}

namespace alive {
class ObjectCommand : public Command
{
public:
    ObjectCommand(model::Object *object, CommandType type, const std::string &name)
        : Command(type, name)
        , m_object(object)
    {}

    const model::Object *object() const { return m_object; }

protected:
    model::Object *m_object;
};

class SetObjectBooleanCommand : public ObjectCommand
{
public:
    enum Type { e_Visible };
    SetObjectBooleanCommand(Type type,
                            bool flag,
                            model::Object *object,
                            const std::string &name = {});
    void redo() override;
    void undo() override;
    bool flag() const { return m_flag; }
    Type type() const { return m_type; }

private:
    void update_text();
    void apply_change(bool flag);

private:
    Type m_type = e_Visible;
    bool m_flag = true;
};

} // namespace alive
#endif // OBJECT_COMMANDS_H
