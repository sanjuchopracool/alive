#include "tab_widget.h"
#include "utility.h"
#include <QHBoxLayout>
#include <QPushButton>
#include <QTabWidget>

namespace alive {

TabWidget::TabWidget(QWidget *parent)
    : DockWidget(parent)
{
    QHBoxLayout *main_layout = hbox_layout(1);
    m_tab = new QTabWidget(this);
    main_layout->addWidget(m_tab);
    setLayout(main_layout);
}

TabWidget::~TabWidget() {}

} // namespace alive
