#ifndef TIMELINE_TAB_WIDGET_H
#define TIMELINE_TAB_WIDGET_H

#include "tab_widget.h"

namespace alive::model {
class CompositionDocument;
class Layer;
}

namespace alive {
struct TimelineTabWidgetPrivateData;
class TimelineTabWidget : public TabWidget
{
    Q_OBJECT
public:
    TimelineTabWidget(QWidget *parent);
    ~TimelineTabWidget();

    void open_tab_for_doc(alive::model::CompositionDocument *doc);
    void close_tab_for_doc(alive::model::CompositionDocument *doc);
    void select_tab_for_doc(alive::model::CompositionDocument *doc);

signals:
    void selected_tab_for_doc(alive::model::CompositionDocument *doc);
    void layer_selected(alive::model::Layer *layer);
};
} // namespace alive

#endif // TIMELINE_TAB_WIDGET_H
