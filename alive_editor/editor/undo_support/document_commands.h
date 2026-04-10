#ifndef DOCUMENT_COMMANDS_H
#define DOCUMENT_COMMANDS_H

#include <editor/undo_support/command.h>
#include <editor/settings/composition_setting.h>

namespace alive {
class CompositionSettingCommand : public Command
{
public:
    CompositionSettingCommand(const CompositionSetting &setting);

    void redo() override;
    void undo() override;
    bool merge_with(const Command *other) override;

    const CompositionSetting &old_setting() const { return m_old_setting; }
    const CompositionSetting &setting() const { return m_setting; }

private:
    void update_document();

private:
    CompositionSetting m_setting;
    CompositionSetting m_old_setting;
};
} // namespace alive::commands

#endif // DOCUMENT_COMMANDS_H
