#ifndef BEZIER_CURVE_EDITOR_H
#define BEZIER_CURVE_EDITOR_H

#include <core/model/property/easing.h>
#include <QPainterPath>
#include <QWidget>

namespace alive::model {
class KeyFrame;
}

namespace alive {

class BezierCurveEditor : public QWidget
{
    Q_OBJECT
public:
    explicit BezierCurveEditor(QWidget *parent = nullptr);

    void set_keyframes(alive::model::KeyFrame *left, alive::model::KeyFrame *right);
    void set_margin(int value);
    int margin() const { return m_margin; }
    void reset_zoom();
signals:
    void easing_data_changed(alive::model::EasingBetweenTwoFramesData *data);

protected:
    void keyPressEvent(QKeyEvent *ev) override;
    void paintEvent(QPaintEvent *ev) override;
    void wheelEvent(QWheelEvent *ev) override;

    // move events
    void mousePressEvent(QMouseEvent *ev) override;
    void mouseReleaseEvent(QMouseEvent *ev) override;
    void mouseMoveEvent(QMouseEvent *ev) override;

private:
    void update_transform();

    enum ClickPoint {
        e_None,
        e_CT1,
        e_CT2,
    };

private:
    int m_margin = 0;
    qreal m_zoom = 1.0f;
    ClickPoint m_clicked_point = ClickPoint::e_None;
    QPointF m_last_pos;
    alive::model::EasingBetweenTwoFramesData m_data;
};

} // namespace alive

#endif // BEZIER_CURVE_EDITOR_H
