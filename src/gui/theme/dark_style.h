#ifndef DARKSTYLE_H
#define DARKSTYLE_H

#include <QPalette>
#include <QProxyStyle>

namespace inae::theme {
class DarkStyle : public QProxyStyle
{
    Q_OBJECT
public:
    DarkStyle();
    QPalette standardPalette() const override;

private:
    mutable QPalette m_standardPalette;
};
} // namespace inae::theme

#endif // DARKSTYLE_H
