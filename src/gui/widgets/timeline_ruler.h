#ifndef TIMELINE_RULER_WIDGET_H
#define TIMELINE_RULER_WIDGET_H

#include <QPixmap>
#include <QWidget>

namespace alive::gui {
class TimelineHelper;
} // namespace alive::gui

namespace alive {
class TimelineRuler : public QWidget
{
    Q_OBJECT
public:
    TimelineRuler(alive::gui::TimelineHelper *helper, QWidget *parent = nullptr);
    int range_start() const { return m_range_start; }
    void set_range_start(int range);

    int range_end() const { return m_range_end; }
    void set_range_end(int range);

    qreal fps() const { return m_fps; }
    const void set_fps(qreal fps);

    qreal ruler_tick_height() const { return m_ruler_tick_height; }
    void set_ruler_tick_height(qreal height)
    {
        if (m_ruler_tick_height != height) {
            m_ruler_tick_height = height;
            emit ruler_tick_height_changed();
        }
    }

    qreal handle_width() const { return m_handle_width; }
    void set_handle_width(qreal width)
    {
        if (m_handle_width != width) {
            m_handle_width = width;
            emit handle_width_changed();
        }
    }

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *ev) override;
    void resizeEvent(QResizeEvent *ev) override;
    void mousePressEvent(QMouseEvent* ev) override;
    void mouseMoveEvent(QMouseEvent* ev) override;

private:
    void update_pixmap();

signals:
    void range_start_changed();
    void range_end_changed();
    void text_color_changed();
    void fps_changed();
    void ruler_tick_height_changed();
    void handle_width_changed();

private:
    alive::gui::TimelineHelper *m_helper = nullptr;
    int m_range_start = 0;
    int m_range_end = 30;
    qreal m_handle_width = 10;
    qreal m_fps = 30;
    QPixmap m_ruler_pixmap;
    qreal m_ruler_tick_height = 10;
};
} // namespace Alive

#endif // TIMELINE_RULER_WIDGET_H
