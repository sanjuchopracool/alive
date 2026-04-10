#include "bezier_curve_editor.h"
#include <core/model/keyframes/keyframe.h>
#include <gui/framework/theme/color_palette.h>
#include <QKeyEvent>
#include <QPainter>
#include <QVector2D>
#include <QWheelEvent>

namespace alive {

BezierCurveEditor::BezierCurveEditor(QWidget *parent)
    : QWidget{parent}
{
    setFocusPolicy(Qt::StrongFocus);
}

void BezierCurveEditor::set_keyframes(alive::model::KeyFrame *left, alive::model::KeyFrame *right)
{
    m_data.m_left_keyframe = left;
    m_data.m_right_keyframe = right;
    m_data.m_ct1 = Vec2D(left->out_tangent().x(), left->out_tangent().y());
    m_data.m_ct2 = Vec2D(right->in_tangent().x(), right->in_tangent().y());
    update();
}

void BezierCurveEditor::set_margin(int value)
{
    if (m_margin != value) {
        m_margin = value;
        update();
    }
}

void BezierCurveEditor::reset_zoom()
{
    m_zoom = 1.0f;
}

void BezierCurveEditor::keyPressEvent(QKeyEvent *ev)
{
    if (ev->key() == Qt::Key_Escape && ev->modifiers() == Qt::NoModifier) {
        this->hide();
        ev->accept();
        return;
    }
    QWidget::keyPressEvent(ev);
}

void BezierCurveEditor::paintEvent(QPaintEvent *ev)
{
    auto colors = alive::theme::ColorPalette::instance();
    QPainter painter(this);
    painter.fillRect(this->rect(), colors->primary_background_color());
    QPainterPath path;
    auto rect = this->rect();
    painter.translate(rect.center());

    painter.scale(m_zoom, m_zoom);
    rect = rect.adjusted(m_margin, m_margin, -m_margin, -m_margin);
    qreal scale_x = rect.width();
    qreal scale_y = rect.height();
    painter.translate(-rect.width() / 2, -rect.height() / 2);
    QPointF ct1(m_data.m_ct1.x() * scale_x, scale_y - m_data.m_ct1.y() * scale_y);
    QPointF ct2(m_data.m_ct2.x() * scale_x, scale_y - m_data.m_ct2.y() * scale_y);
    // path.cubicTo()
    QPointF start(0, rect.height());
    path.moveTo(start);
    // path.cubicTo(QPoint{0, rect.height()}, QPoint{rect.width(), 0}, QPointF(rect.width(), 0));
    QPointF end(rect.width(), 0);
    path.cubicTo(ct1, ct2, end);

    qreal inverse_zoom = 1.0f / m_zoom;
    QPen pen(colors->accent_color());
    {
        painter.save();
        pen.setWidthF(2 * inverse_zoom);
        painter.setPen(pen);
        painter.drawPath(path);
        painter.restore();
    }

    // draw control points nad lines to points
    {
        pen.setWidth(1 * inverse_zoom);
        pen.setColor(colors->secondary_font_color());
        painter.setPen(pen);
        painter.drawLine(start, ct1);
        painter.drawLine(ct2, end);

        auto margin = m_margin * inverse_zoom;
        auto half_margin = margin / 2.0;
        QRectF rect(-half_margin, -half_margin, margin, margin);
        painter.setBrush(colors->accent_color());
        rect.moveCenter(start);
        painter.drawEllipse(rect);
        rect.moveCenter(end);
        painter.drawEllipse(rect);

        painter.setPen(Qt::NoPen);
        painter.setBrush(colors->primary_font_color());
        rect.moveCenter(ct1);
        painter.drawEllipse(rect);
        rect.moveCenter(ct2);
        painter.drawEllipse(rect);
    }
}

void BezierCurveEditor::wheelEvent(QWheelEvent *ev)
{
    if (ev->angleDelta().y() > 0.0) {
        m_zoom = m_zoom + 0.15;
    } else {
        m_zoom = m_zoom - 0.15;
    }

    m_zoom = std::clamp(m_zoom, 0.2, 1.0);
    update();
}

void BezierCurveEditor::mousePressEvent(QMouseEvent *ev)
{
    QVector2D pos(ev->position());
    // handle margin
    // map it to coordinates without scale
    auto rect = this->rect();

    pos.setX(pos.x() - rect.center().x());
    pos.setY(pos.y() - rect.center().y());

    qreal inverse_zoom = 1.0f / m_zoom;
    rect = rect.adjusted(m_margin, m_margin, -m_margin, -m_margin);

    pos.setX(pos.x() * inverse_zoom);
    pos.setY(pos.y() * inverse_zoom);

    pos.setX(pos.x() + rect.width() / 2);
    pos.setY(pos.y() + rect.height() / 2);

    qreal scale_x = rect.width();
    qreal scale_y = rect.height();
    QVector2D ct1(m_data.m_ct1.x() * scale_x, scale_y - m_data.m_ct1.y() * scale_y);
    QVector2D ct2(m_data.m_ct2.x() * scale_x, scale_y - m_data.m_ct2.y() * scale_y);

    auto half_margin = m_margin * inverse_zoom / 2.0;
    if (ct1.distanceToPoint(pos) <= half_margin) {
        m_clicked_point = e_CT1;
    } else if (ct2.distanceToPoint(pos) <= half_margin) {
        m_clicked_point = e_CT2;
    }
    m_last_pos = ev->position();
    ev->accept();
}

void BezierCurveEditor::mouseReleaseEvent(QMouseEvent *ev)
{
    m_clicked_point = ClickPoint::e_None;
    ev->accept();
}

void BezierCurveEditor::mouseMoveEvent(QMouseEvent *ev)
{
    if (m_clicked_point != ClickPoint::e_None) {
        QVector2D delta = QVector2D(ev->position() - m_last_pos);
        //scale delta to zoom
        auto rect = this->rect().adjusted(m_margin, m_margin, -m_margin, -m_margin);
        qreal inverse_zoom = 1.0f / m_zoom;
        delta.setX(delta.x() * inverse_zoom / rect.width());
        delta.setY(-delta.y() * inverse_zoom / rect.height());
        Vec2D change(delta.x(), delta.y());

        if (m_clicked_point == ClickPoint::e_CT1) {
            m_data.m_ct1 += change;
        } else {
            m_data.m_ct2 += change;
        }
        m_last_pos = ev->position();
        update();
        emit easing_data_changed(&m_data);
    }
    ev->accept();
}

} // namespace alive
