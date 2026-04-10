#ifndef FILE_BASED_LAYER_SETTING_DIALOG_H
#define FILE_BASED_LAYER_SETTING_DIALOG_H

#include <QDialog>

namespace Ui {
class FileBasedLayerSettingDialog;
}

namespace alive::model {
class FileBasedLayerSetting;
} // namespace alive::model

namespace alive {
class FileBasedLayerSettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FileBasedLayerSettingDialog(model::FileBasedLayerSetting &setting,
                                         QWidget *parent = nullptr);
    ~FileBasedLayerSettingDialog();

signals:
    void setting_changed();

private:
    Ui::FileBasedLayerSettingDialog *m_ui;
    alive::model::FileBasedLayerSetting &m_setting;
};
} // namespace alive
#endif // FILE_BASED_LAYER_SETTING_DIALOG_H
