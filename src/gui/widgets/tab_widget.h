#ifndef TAB_WIDGET_H
#define TAB_WIDGET_H

#include "dock_widget.h"

QT_BEGIN_NAMESPACE
class QTabWidget;
QT_END_NAMESPACE

namespace alive {

class TabWidget : public DockWidget
{
public:
    TabWidget(QWidget *parent);
    ~TabWidget();

protected:
    QTabWidget *m_tab = nullptr;
};

} // namespace alive

#endif // TAB_WIDGET_H
