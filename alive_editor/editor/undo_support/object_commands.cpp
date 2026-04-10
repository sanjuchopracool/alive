#include <editor/undo_support/object_commands.h>
#include <core/model/property/object.h>
#include <editor/document/document.h>
#include <editor/undo_support/command_enums.h>
#include <format>

namespace {
constexpr char k_visibility_message[] = "Set {} visibility to {}";
constexpr char k_true[] = "true";
constexpr char k_false[] = "false";
} // namespace

namespace alive {

SetObjectBooleanCommand::SetObjectBooleanCommand(Type type,
                                                 bool flag,
                                                 model::Object *object,
                                                 const std::string &text)
    : ObjectCommand(object, CommandType::e_ObjectSetBool, text)
    , m_type(type)
    , m_flag(flag)
{
    if (text.empty()) {
        update_text();
    }
}

void SetObjectBooleanCommand::redo()
{
    apply_change(m_flag);
}

void SetObjectBooleanCommand::undo()
{
    apply_change(!m_flag);
}

void SetObjectBooleanCommand::update_text()
{
    if (m_type == e_Visible) {
        m_name = std::format(k_visibility_message, m_object->name(), m_flag ? k_true : k_false);
    }
}

void SetObjectBooleanCommand::apply_change(bool flag)
{
    switch (m_type) {
    case e_Visible:
        m_object->set_object_visible(flag);
        break;
    default:
        break;
    }
    if (m_document) {
        m_document->emit_updated_item(m_object, m_roles);
    }
}

} // namespace alive
