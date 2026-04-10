#ifndef COMPOSITION_SETTING_DIALOG_H
#define COMPOSITION_SETTING_DIALOG_H

#include <QDialog>

namespace Ui {
class CompositionSettingDialog;
}

namespace alive {
class CompositionSetting;
} // namespace alive::model

namespace alive {
class CompositionSettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CompositionSettingDialog(alive::CompositionSetting &setting,
                                      QWidget *parent = nullptr);
    ~CompositionSettingDialog();

protected:
    void keyPressEvent(QKeyEvent *evt) override;

private:
    Ui::CompositionSettingDialog *m_ui;
    alive::CompositionSetting &m_setting;
};
} // namespace alive

#endif // COMPOSITION_SETTING_DIALOG_H
