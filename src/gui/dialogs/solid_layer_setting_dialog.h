#ifndef SOLID_LAYER_SETTING_H
#define SOLID_LAYER_SETTING_H

#include <QDialog>

namespace Ui {
class SolidLayerSetting;
}

namespace alive::model {
class SolidLayerSetting;
} // namespace alive::model

namespace alive {
class SolidLayerSettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SolidLayerSettingDialog(alive::model::SolidLayerSetting &setting,
                                     QWidget *parent = nullptr);
    ~SolidLayerSettingDialog();

signals:
    void setting_changed();

protected:
    void keyPressEvent(QKeyEvent *evt) override;

private:
    Ui::SolidLayerSetting *m_ui;
    alive::model::SolidLayerSetting &m_setting;
};
} // namespace alive
#endif // SOLID_LAYER_SETTING_H
