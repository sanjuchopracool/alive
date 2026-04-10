#ifndef COMMAND_H
#define COMMAND_H

#include <core/alive_types/class_helpers.h>
#include <string>
#include <vector>

namespace alive {

enum class CommandType;
class Document;
class Command
{
    ONLY_CLONABLE(Command)
public:
    Command(CommandType type, std::string name)
        : m_name(std::move(name))
    {}
    Command(const Command &) = default;
    virtual ~Command() {};

    void set_name(std::string name) { m_name = std::move(name); }
    const std::string &name() const { return m_name; }

    virtual void redo() = 0;
    virtual void undo() = 0;

    virtual bool merge_with(const Command *) { return false; }

    CommandType type() const { return m_cmd_type; }
    bool can_merge(const Command *other) { return m_cmd_type == other->m_cmd_type; }
    void set_document(Document *document) { m_document = document; }
    void set_roles(const std::vector<int> &roles) { m_roles = roles; }

private:
protected:
    std::string m_name;
    CommandType m_cmd_type;
    Document *m_document = nullptr;
    std::vector<int> m_roles;
};

} // namespace alive

#endif // COMMAND_H
