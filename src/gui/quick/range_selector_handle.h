#ifndef RANGESELECTORHANDLE_H
#define RANGESELECTORHANDLE_H

#include <QPainterPath>
#include <QQuickPaintedItem>

namespace alive::gui {
class RangeSelectorHandle : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(bool is_left READ is_left WRITE set_left NOTIFY is_left_changed FINAL)
    Q_PROPERTY(qreal border_width READ border_width WRITE set_border_width NOTIFY
                   border_width_changed FINAL)
    Q_PROPERTY(qreal border_radius READ border_radius WRITE set_border_radius NOTIFY
                   border_radius_changed FINAL)
    Q_PROPERTY(QColor color READ color WRITE set_color NOTIFY color_changed FINAL)
    Q_PROPERTY(QColor border_color READ border_color WRITE set_border_color NOTIFY
                   border_color_changed FINAL)

public:
    RangeSelectorHandle(QQuickItem *parent = nullptr);
    void paint(QPainter *painter) override;

    bool is_left() const { return m_is_left; }
    void set_left(bool flag)
    {
        if (m_is_left != flag) {
            m_is_left = flag;
            emit is_left_changed();
        }
    }

    qreal border_width() const { return m_border_width; }
    void set_border_width(qreal width)
    {
        if (m_border_width != width) {
            m_border_width = width;
            emit border_width_changed();
        }
    }

    qreal border_radius() const { return m_border_radius; }
    void set_border_radius(qreal radius);

    QColor color() const { return m_color; }
    void set_color(QColor color)
    {
        if (m_color != color) {
            m_color = color;
            emit color_changed();
        }
    }

    QColor border_color() const { return m_border_color; }
    void set_border_color(QColor color)
    {
        if (m_border_color != color) {
            m_border_color = color;
            emit border_color_changed();
        }
    }

private:
    void update_path();

signals:
    void is_left_changed();
    void border_width_changed();
    void border_radius_changed();
    void color_changed();
    void border_color_changed();

private:
    bool m_is_left = true;
    qreal m_border_width = 1;
    qreal m_border_radius = 4;
    QColor m_color;
    QColor m_border_color = Qt::black;
    QPainterPath m_path;
};
} // namespace alive::gui

#endif // RANGESELECTORHANDLE_H
