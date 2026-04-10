#ifndef FONT_SETTING_WIDGET_H
#define FONT_SETTING_WIDGET_H

#include <core/font.h>
#include <QWidget>

namespace Ui {
class FontSettingWidget;
}

namespace alive {
class FontSettingWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FontSettingWidget(QWidget *parent = nullptr);
    ~FontSettingWidget();
    void set_font(core::Font &font);

    const core::Font &font() const { return m_font; }
    void set_font(const core::Font &font);

signals:
    void font_changed(const core::Font &);

private:
    void update_ui();

private:
    Ui::FontSettingWidget *m_ui;
    core::Font m_font;
};
} // namespace alive
#endif // FONT_SETTING_WIDGET_H
