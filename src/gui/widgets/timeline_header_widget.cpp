#include "timeline_header_widget.h"
#include "timeline_headerview.h"
#include <gui/widgets/utility.h>
#include <QLineEdit>

namespace alive {

TimelineHeaderWidget::TimelineHeaderWidget(QWidget *parent)
    : QWidget{parent}
{
    auto filter = new QLineEdit(this);
    filter->setPlaceholderText(tr("Filter"));

    m_header_view = new TimelineHeaderView(this);
    m_header_view->setMinimumWidth(200);

    QVBoxLayout *layout = vbox_layout();
    layout->addWidget(filter);
    layout->addWidget(m_header_view);
    setLayout(layout);

    connect(filter, &QLineEdit::textChanged, this, &TimelineHeaderWidget::filter_changed);
}

} // namespace alive
