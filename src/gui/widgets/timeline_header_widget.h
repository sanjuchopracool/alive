#ifndef TIMELINE_HEADER_WIDGET_H
#define TIMELINE_HEADER_WIDGET_H

#include <QWidget>

namespace alive {
class TimelineHeaderView;
class TimelineHeaderWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TimelineHeaderWidget(QWidget *parent = nullptr);
    TimelineHeaderView *header_view() const { return m_header_view; }

signals:
    void filter_changed(QString);

private:
    TimelineHeaderView *m_header_view = nullptr;
};
} // namespace alive

#endif // TIMELINE_HEADER_WIDGET_H
