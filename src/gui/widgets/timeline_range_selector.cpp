#include "timeline_range_selector.h"
#include "timeline_range_selector_item.h"
#include "timeline_ruler.h"
#include "utility.h"
#include <gui/quick/range_selector_helper.h>
#include <gui/theme/timeline_style.h>
#include <QResizeEvent>

namespace alive {
TimelineRangeSelector::TimelineRangeSelector(alive::gui::TimeLine *timeline, QWidget *parent)
    : QWidget{parent}
{
    m_helper = new alive::gui::TimelineHelper(timeline, this);
    m_ruler = new TimelineRuler(m_helper, this);
    m_visible_range_selector
        = new TimelineRangeSelectorItem(m_helper, TimelineRangeSelectorItem::e_VisibleRange, this);
    m_play_range_selector = new TimelineRangeSelectorItem(m_helper,
                                                          TimelineRangeSelectorItem::e_PlayRange,
                                                          this);
    QVBoxLayout *layout = vbox_layout();
    layout->addWidget(m_visible_range_selector);
    layout->addWidget(m_ruler);
    layout->addWidget(m_play_range_selector);
    setLayout(layout);

    update_look_and_feel();
}

QSize TimelineRangeSelector::sizeHint() const
{
    return QSize(400,
                 m_visible_range_selector->height() + m_ruler->height()
                     + m_play_range_selector->height());
}

void TimelineRangeSelector::resizeEvent(QResizeEvent *ev)
{
    QWidget::resizeEvent(ev);
}

void TimelineRangeSelector::update_look_and_feel()
{
    const auto *timeline_style = alive::theme::TimelineSize::instance();
    m_visible_range_selector->setFixedHeight(timeline_style->visible_range_selector_height());
    m_ruler->setFixedHeight(timeline_style->ruler_height());
    m_play_range_selector->setFixedHeight(timeline_style->play_range_selector_height());
    setFixedHeight(m_visible_range_selector->height() + m_ruler->height()
                   + m_play_range_selector->height());
}
} // namespace Alive
