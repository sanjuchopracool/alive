#ifndef ALIVE_STYLE_H
#define ALIVE_STYLE_H

#include <QProxyStyle>

namespace alive::theme {

class Style : public QProxyStyle
{
public:
    Style(QStyle *style);

protected:
    void polish(QPalette &pl) override;
    void drawControl(QStyle::ControlElement element,
                     const QStyleOption *option,
                     QPainter *painter,
                     const QWidget *widget = nullptr) const override;
};

} // namespace alive::theme

#endif // ALIVE_STYLE_H
