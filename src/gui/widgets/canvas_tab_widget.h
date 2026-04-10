#ifndef CANVAS_TAB_WIDGET_H
#define CANVAS_TAB_WIDGET_H

#include "tab_widget.h"

namespace alive::model {
class CompositionDocument;
}

namespace alive {
class CanvasTabWidget : public TabWidget
{
    Q_OBJECT
public:
    CanvasTabWidget(QWidget *parent);

    void open_tab_for_doc(alive::model::CompositionDocument *doc);
    void close_tab_for_doc(alive::model::CompositionDocument *doc);
    void select_tab_for_doc(alive::model::CompositionDocument *doc);

    void zoom_in();
    void zoom_out();
    void zoom_fit();

signals:
    void selected_tab_for_doc(alive::model::CompositionDocument *doc);
};

} // namespace alive

#endif // CANVAS_TAB_WIDGET_H
