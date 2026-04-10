#ifndef TIMELINE_WIDGET_H
#define TIMELINE_WIDGET_H

#include <memory>
#include <QWidget>

namespace alive::model {
class CompositionDocument;
class Layer;
}

namespace alive {

class TimelineWidgetData;
class TimelineWidget : public QWidget
{
    Q_OBJECT
public:
    TimelineWidget(QWidget *parent, alive::model::CompositionDocument *doc);
    ~TimelineWidget();

    alive::model::CompositionDocument *document() const;

signals:
    void layer_selected(alive::model::Layer *layer);

protected:
    void resizeEvent(QResizeEvent *ev) override;
    void showEvent(QShowEvent *ev) override;

private slots:
    void update_play_indicator();
    void update_selected_playhead();
    void update_column_size();

private:
    void restore_settings();
    void save_settings();
    void update_element_size();

private:
    std::unique_ptr<TimelineWidgetData> m_d;
};

} // namespace alive
#endif // TIMELINE_WIDGET_H
