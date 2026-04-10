#include <editor/document/document.h>
#include <editor/undo_support/command_enums.h>
#include <editor/undo_support/document_commands.h>
#include <editor/undo_support/document_undo_manager.h>
namespace alive {

CompositionSettingCommand::CompositionSettingCommand(const CompositionSetting &setting)
    : Command(CommandType::e_CompositionSetting, "Composition Settings")
    , m_setting(setting)
{}

void CompositionSettingCommand::redo()
{
    if (m_document) {
        m_old_setting = m_document->composition_setting();
        m_document->undo_manager()->update_composition_setting_impl(m_setting);
    }
}

void CompositionSettingCommand::undo()
{
    if (m_document) {
        m_document->undo_manager()->update_composition_setting_impl(m_old_setting);
    }
}

bool CompositionSettingCommand::merge_with(const Command *other)
{
    const CompositionSettingCommand *other_command = static_cast<const CompositionSettingCommand *>(
        other);
    if (m_setting == other_command->old_setting()) {
        m_setting = other_command->setting();
        return true;
    }

    return false;
}

} // namespace alive::commands
