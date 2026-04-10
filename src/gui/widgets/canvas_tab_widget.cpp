#include "canvas_tab_widget.h"
#include <gui/document/composition_document.h>
#include <gui/widgets/canvas_widget.h>
#include <QTabWidget>

namespace alive {

CanvasTabWidget::CanvasTabWidget(QWidget *parent)
    : TabWidget(parent)
{
    connect(m_tab, &QTabWidget::currentChanged, this, [this]() {
        CanvasWidget *w = qobject_cast<CanvasWidget *>(m_tab->currentWidget());
        if (w) {
            emit selected_tab_for_doc(w->document());
        }
    });
}

void CanvasTabWidget::open_tab_for_doc(alive::model::CompositionDocument *doc)
{
    CanvasWidget *canvas = new CanvasWidget(doc, this);
    m_tab->addTab(canvas, doc->name());
}

void CanvasTabWidget::close_tab_for_doc(alive::model::CompositionDocument *doc)
{
    int count = m_tab->count();
    for (int i = 0; i < count; ++i) {
        CanvasWidget *w = qobject_cast<CanvasWidget *>(m_tab->widget(i));
        if (w && w->document() == doc) {
            QSignalBlocker b(m_tab);
            m_tab->removeTab(i);
            delete w;
        }
    }
}

void CanvasTabWidget::select_tab_for_doc(alive::model::CompositionDocument *doc)
{
    int count = m_tab->count();
    for (int i = 0; i < count; ++i) {
        CanvasWidget *w = qobject_cast<CanvasWidget *>(m_tab->widget(i));
        if (w && w->document() == doc) {
            QSignalBlocker b(m_tab);
            m_tab->setCurrentIndex(i);
        }
    }
}

void CanvasTabWidget::zoom_in()
{
    CanvasWidget *w = qobject_cast<CanvasWidget *>(m_tab->currentWidget());
    if (w)
        w->zoom_in();
}

void CanvasTabWidget::zoom_out()
{
    CanvasWidget *w = qobject_cast<CanvasWidget *>(m_tab->currentWidget());
    if (w)
        w->zoom_out();
}

void CanvasTabWidget::zoom_fit()
{
    CanvasWidget *w = qobject_cast<CanvasWidget *>(m_tab->currentWidget());
    if (w)
        w->zoom_to_fit();
}

} // namespace alive
