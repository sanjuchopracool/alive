#ifndef SCRIPT_EDITOR_DIALOG_H
#define SCRIPT_EDITOR_DIALOG_H

#include <QDialog>

namespace Ui {
class ScriptEditorDialog;
}

namespace alive {
class ScriptEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ScriptEditorDialog(const std::string &script, QWidget *parent = nullptr);
    ~ScriptEditorDialog();
    std::string script_text() const;

private:
    Ui::ScriptEditorDialog *ui;
    QString m_initial_script;
    bool m_applied = false;
};
} // namespace alive

#endif // SCRIPT_EDITOR_DIALOG_H
