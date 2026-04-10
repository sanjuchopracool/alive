#ifndef TIMELINE_RULER_H
#define TIMELINE_RULER_H

#include <QPixmap>
#include <QQuickPaintedItem>

namespace alive::gui {
class TimelineRuler : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(
        int range_start READ range_start WRITE set_range_start NOTIFY range_start_changed FINAL)
    Q_PROPERTY(int range_end READ range_end WRITE set_range_end NOTIFY range_end_changed FINAL)
    Q_PROPERTY(QColor text_color READ text_color WRITE set_text_color NOTIFY text_color_changed FINAL)
    Q_PROPERTY(qreal fps READ fps WRITE set_fps NOTIFY fps_changed FINAL)
    Q_PROPERTY(qreal handle_width READ handle_width WRITE set_handle_width NOTIFY
                   handle_width_changed FINAL)
    Q_PROPERTY(qreal ruler_tick_height READ ruler_tick_height WRITE set_ruler_tick_height NOTIFY
                   ruler_tick_height_changed FINAL)

public:
    TimelineRuler(QQuickItem *parent = nullptr);
    void paint(QPainter *painter) override;

    int range_start() const { return m_range_start; }
    void set_range_start(int range);

    int range_end() const { return m_range_end; }
    void set_range_end(int range);

    QColor text_color() const { return m_text_color; }
    void set_text_color(QColor text_color)
    {
        if (m_text_color != text_color) {
            m_text_color = text_color;
            emit text_color_changed();
        }
    }

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
    int m_range_start = 0;
    int m_range_end = 30;
    qreal m_handle_width = 10;
    qreal m_fps = 30;
    QColor m_text_color;
    QPixmap m_ruler_pixmap;
    qreal m_ruler_tick_height = 10;
};
} // namespace alive::gui

#endif // TIMELINE_RULER_H
