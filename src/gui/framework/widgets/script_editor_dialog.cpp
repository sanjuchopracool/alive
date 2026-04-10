#include "script_editor_dialog.h"
#include "ui_script_editor_dialog.h"
#include <gui/project/project_manager.h>

namespace alive {
ScriptEditorDialog::ScriptEditorDialog(const std::string &script, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ScriptEditorDialog)
    , m_initial_script(QString::fromStdString(script))
{
    ui->setupUi(this);
    ui->text_edit->setPlainText(m_initial_script);
    QPalette palette = ui->error_label->palette();
    palette.setColor(ui->error_label->foregroundRole(), Qt::red);
    ui->error_label->setPalette(palette);
    ui->error_label->hide();

    const QString verify_str(tr("Verify"));
    ui->btn_apply->setText(verify_str);

    connect(ui->btn_cancel, &QPushButton::clicked, this, [this]() {
        if (m_applied) {
            project::ProjectManager::instance()->is_valid_project_expression(m_initial_script);
        }

        reject();
    });
    connect(ui->text_edit, &QPlainTextEdit::textChanged, this, [this, verify_str]() {
        ui->btn_apply->setText(verify_str);
    });
    connect(ui->btn_apply, &QPushButton::clicked, this, [this, verify_str]() {
        m_applied = true;
        const QString apply_str(tr("Apply"));
        if (ui->btn_apply->text() == verify_str) {
            const auto &[result, error]
                = project::ProjectManager::instance()->is_valid_project_expression(
                    ui->text_edit->toPlainText());
            if (result) {
                ui->btn_apply->setText(apply_str);
                ui->error_label->hide();
            } else {
                ui->error_label->setText(QString::fromStdString(error));
                ui->error_label->show();
            }
        } else if (ui->btn_apply->text() == apply_str) {
            accept();
        }
    });

    setWindowTitle(tr("Update Composition Apply Script"));
}

ScriptEditorDialog::~ScriptEditorDialog()
{
    delete ui;
}

std::string ScriptEditorDialog::script_text() const
{
    return ui->text_edit->toPlainText().toStdString();
}
} // namespace alive
