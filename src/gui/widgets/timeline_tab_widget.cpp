#include "timeline_tab_widget.h"
#include <gui/document/composition_document.h>
#include <gui/project/project_manager.h>
#include <gui/widgets/timeline_widget.h>
#include <QMenu>
#include <QTabWidget>

namespace alive {

TimelineTabWidget::TimelineTabWidget(QWidget *parent)
    : TabWidget(parent)
{
    connect(m_tab, &QTabWidget::currentChanged, this, [this]() {
        TimelineWidget *w = qobject_cast<TimelineWidget *>(m_tab->currentWidget());
        if (w) {
            emit selected_tab_for_doc(w->document());
        }
    });
}

TimelineTabWidget::~TimelineTabWidget() {}

void TimelineTabWidget::open_tab_for_doc(alive::model::CompositionDocument *doc)
{
    TimelineWidget *timeline_widget = new TimelineWidget(this, doc);
    m_tab->addTab(timeline_widget, doc->name());
    // m_tab->setCurrentIndex(m_tab->count() - 1);
    connect(timeline_widget,
            &TimelineWidget::layer_selected,
            this,
            &TimelineTabWidget::layer_selected);
}

void TimelineTabWidget::close_tab_for_doc(alive::model::CompositionDocument *doc)
{
    int count = m_tab->count();
    for (int i = 0; i < count; ++i) {
        TimelineWidget *w = qobject_cast<TimelineWidget *>(m_tab->widget(i));
        if (w && w->document() == doc) {
            m_tab->removeTab(i);
            delete w;
        }
    }
}

void TimelineTabWidget::select_tab_for_doc(alive::model::CompositionDocument *doc)
{
    int count = m_tab->count();
    for (int i = 0; i < count; ++i) {
        TimelineWidget *w = qobject_cast<TimelineWidget *>(m_tab->widget(i));
        if (w && w->document() == doc) {
            QSignalBlocker b(m_tab);
            m_tab->setCurrentIndex(i);
        }
    }
}
} // namespace alive
