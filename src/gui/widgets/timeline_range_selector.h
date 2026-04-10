#ifndef TIMELINE_RANGE_SELECTOR_H
#define TIMELINE_RANGE_SELECTOR_H

#include "alive_widget.h"
#include "timeline_range_selector_item.h"
#include <QWidget>

namespace alive::gui {
class TimelineHelper;
class TimeLine;
} // namespace alive::gui

namespace alive {

class TimelineRuler;
class TimelineRangeSelectorItem;

class TimelineRangeSelector : public QWidget, public WidgetInterface
{
    Q_OBJECT
public:
    explicit TimelineRangeSelector(alive::gui::TimeLine *timeline, QWidget *parent = nullptr);

    QSize sizeHint() const override;

    const alive::gui::TimelineHelper *timeline_helper() const { return m_helper; };

protected:
    void resizeEvent(QResizeEvent *ev) override;

private:
    void update_look_and_feel();

private:
    alive::gui::TimelineHelper *m_helper = nullptr;
    TimelineRuler *m_ruler = nullptr;
    TimelineRangeSelectorItem *m_visible_range_selector = nullptr;
    TimelineRangeSelectorItem *m_play_range_selector = nullptr;
};
} // namespace Alive

#endif // TIMELINE_RANGE_SELECTOR_H
