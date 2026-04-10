#ifndef TIMELINE_HEADERVIEW_H
#define TIMELINE_HEADERVIEW_H

#include <QWidget>

namespace alive {

struct TimelineHeaderViewPrivateData;
class TimelineHeaderView : public QWidget
{
    Q_OBJECT
public:
    TimelineHeaderView(QWidget *parent = nullptr);
    ~TimelineHeaderView();

    int no_of_columns() const;
    int width_for_column(int index) const;
    int handle_width() const;
    void restore_settings();
    void show_matte_details(bool show);

protected:
    void paintEvent(QPaintEvent *ev);

signals:
    void column_resized();
    void show_matte_info_changed(bool, int);

private:
    void save_settings();

private:
    std::unique_ptr<TimelineHeaderViewPrivateData> m_d;
};

} // namespace alive

#endif // TIMELINE_HEADERVIEW_H
