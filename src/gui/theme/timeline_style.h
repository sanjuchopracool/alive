#ifndef TIMELINE_STYLE_H
#define TIMELINE_STYLE_H

#include <QColor>
#include <QPixmap>

#include <QObject>

QT_BEGIN_NAMESPACE
class QQmlEngine;
class QJSEngine;
QT_END_NAMESPACE

namespace alive::theme {
class TimelineSize : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(TimelineSize)
    Q_PROPERTY(
        qreal visible_range_selector_height READ visible_range_selector_height WRITE
            set_visible_range_selector_height NOTIFY visible_range_selector_height_changed FINAL)
    Q_PROPERTY(qreal ruler_height READ ruler_height WRITE set_ruler_height NOTIFY
                   ruler_height_changed FINAL)
    Q_PROPERTY(qreal play_range_selector_height READ play_range_selector_height WRITE
                   set_play_range_selector_height NOTIFY play_range_selector_height_changed FINAL)
    Q_PROPERTY(qreal handle_width READ handle_width WRITE set_handle_width NOTIFY
                   handle_width_changed FINAL)
    Q_PROPERTY(qreal handle_radius READ handle_radius WRITE set_handle_radius NOTIFY
                   handle_radius_changed FINAL)
    Q_PROPERTY(qreal stroke_width READ stroke_width WRITE set_stroke_width NOTIFY
                   stroke_width_changed FINAL)
    Q_PROPERTY(qreal ruler_tick_height READ ruler_tick_height WRITE set_ruler_tick_height NOTIFY
                   ruler_tick_height_changed FINAL)
    Q_PROPERTY(QSize playhead_size READ playhead_size WRITE set_playhead_size NOTIFY
                   playhead_size_changed FINAL)

    Q_PROPERTY(int item_height MEMBER m_item_height NOTIFY item_height_changed FINAL)
    Q_PROPERTY(QSize icon_size MEMBER m_icon_size NOTIFY icon_size_changed FINAL)
    Q_PROPERTY(int item_spacing MEMBER m_item_spacing NOTIFY item_spacing_changed FINAL)
    Q_PROPERTY(int tree_indentation MEMBER m_tree_indentation NOTIFY tree_indentation_changed FINAL)

public:
    qreal visible_range_selector_height() const { return m_visible_range_selector_height; }
    void set_visible_range_selector_height(qreal height)
    {
        if (m_visible_range_selector_height != height) {
            m_visible_range_selector_height = height;
            emit play_range_selector_height_changed();
        }
    }

    qreal ruler_height() const { return m_ruler_height; }
    void set_ruler_height(qreal height)
    {
        if (m_ruler_height != height) {
            m_ruler_height = height;
            emit ruler_height_changed();
        }
    }

    qreal play_range_selector_height() const { return m_play_range_selector_height; }
    void set_play_range_selector_height(qreal height)
    {
        if (m_play_range_selector_height != height) {
            m_play_range_selector_height = height;
            emit visible_range_selector_height_changed();
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

    qreal stroke_width() const { return m_stroke_width; }
    void set_stroke_width(qreal width)
    {
        if (m_stroke_width != width) {
            m_stroke_width = width;
            emit stroke_width_changed();
        }
    }

    qreal handle_radius() const { return m_handle_radius; }
    void set_handle_radius(qreal radius)
    {
        if (m_handle_radius != radius) {
            m_handle_radius = radius;
            emit handle_radius_changed();
        }
    }

    QSize playhead_size() const { return m_playhead_size; }
    void set_playhead_size(QSize size)
    {
        if (m_playhead_size != size) {
            m_playhead_size = size;
            emit playhead_size_changed();
        }
    }

    qreal ruler_tick_height() const { return m_ruler_tick_height; }
    void set_ruler_tick_height(qreal height)
    {
        if (m_ruler_tick_height != height) {
            m_ruler_tick_height = height;
            emit ruler_tick_height_changed();
        }
    }

    static QObject *qml_instance(QQmlEngine *engine, QJSEngine *script_engine)
    {
        Q_UNUSED(engine);
        Q_UNUSED(script_engine);
        return instance();
    }

    static void init();
    static void deinit(bool qml = false);
    static TimelineSize *instance();

    const QSize &icon_size() const { return m_icon_size; }
    int item_height() const { return m_item_height; }
    int item_spacing() const { return m_item_spacing; }
    int column_width() const { return 80; }
signals:
    void visible_range_selector_height_changed();
    void ruler_height_changed();
    void play_range_selector_height_changed();
    void handle_width_changed();
    void handle_radius_changed();
    void stroke_width_changed();
    void ruler_tick_height_changed();
    void playhead_size_changed();
    void item_height_changed();
    void icon_size_changed();
    void item_spacing_changed();
    void tree_indentation_changed();

private:
    explicit TimelineSize(QObject *parent = nullptr);

private:
    qreal m_visible_range_selector_height = 16;
    qreal m_ruler_height = 20;
    qreal m_play_range_selector_height = 16;
    qreal m_handle_width = 8;
    qreal m_handle_radius = 4;
    qreal m_stroke_width = 1;
    qreal m_ruler_tick_height = 10;
    QSize m_playhead_size = {16, 16};
    int m_item_height = 18;
    QSize m_icon_size{12, 12};
    int m_item_spacing = 5;
    int m_tree_indentation = 15;
};
} // namespace alive::theme

#endif // TIMELINE_STYLE_H
