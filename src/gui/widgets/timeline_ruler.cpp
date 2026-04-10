#include "timeline_ruler.h"
#include <gui/composition/timeline.h>
#include <gui/quick/range_selector_helper.h>
#include <gui/framework/theme/color_palette.h>
#include <QFont>
#include <QFontMetrics>
#include <QGuiApplication>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>

namespace alive {
TimelineRuler::TimelineRuler(alive::gui::TimelineHelper *helper, QWidget *parent)
    : QWidget(parent)
    , m_helper(helper)
{
    setMouseTracking(false);
    auto *timeline = helper->timeline();
    auto update_range = [this, timeline]() {
        if (m_range_start != timeline->range_start() || m_range_end != timeline->range_end()) {
            m_range_start = timeline->range_start();
            m_range_end = timeline->range_end();
            update_pixmap();
            update();
            emit range_start_changed();
            emit range_end_changed();
        }
    };

    connect(timeline, &alive::gui::TimeLine::range_changed, this, update_range);
    auto update_fps = [this, timeline]() {
        if (m_fps != timeline->fps()) {
            m_fps = timeline->fps();
            update_pixmap();
            update();
            emit fps_changed();
        }
    };

    connect(timeline, &alive::gui::TimeLine::fps_changed, this, update_fps);

    update_range();
    update_fps();
}

void TimelineRuler::set_range_start(int range)
{
    if (m_range_start != range) {
        m_range_start = range;
        update_pixmap();
        update();
        emit range_start_changed();
    }
}

void TimelineRuler::set_range_end(int range)
{
    if (m_range_end != range) {
        m_range_end = range;
        update_pixmap();
        update();
        emit range_end_changed();
    }
}

const void TimelineRuler::set_fps(qreal fps)
{
    if (m_fps != fps) {
        m_fps = fps;
        update_pixmap();
        update();
        emit fps_changed();
    }
}

QSize TimelineRuler::sizeHint() const
{
    return QSize(400, 100);
}

void TimelineRuler::paintEvent(QPaintEvent *ev)
{
    Q_UNUSED(ev)
    QPainter painter(this);
    painter.drawPixmap(this->rect(), m_ruler_pixmap);
}

void TimelineRuler::resizeEvent(QResizeEvent *ev)
{
    Q_UNUSED(ev)
    int new_width = ev->size().width();
    m_helper->set_width(new_width);
    update_pixmap();
    QWidget::resizeEvent(ev);
}

void TimelineRuler::mousePressEvent(QMouseEvent *ev)
{
    m_helper->select_frame_for_x(ev->pos().x());
    QWidget::mousePressEvent(ev);
}

void TimelineRuler::mouseMoveEvent(QMouseEvent *ev)
{
    m_helper->select_frame_for_x(ev->pos().x());
    QWidget::mouseMoveEvent(ev);
}

void TimelineRuler::update_pixmap()
{
    if (!width())
        return;

    // qDebug() << "Ruler Width: " << width();
    // qreal effective_width = width() - 2 * m_handle_width;
    qreal pixel_ratio = devicePixelRatioF();
    m_ruler_pixmap = QPixmap(size() * pixel_ratio);
    //    qDebug() << "Ruler Pixmap Width: " << m_ruler_pixmap.size().width();
    const auto *color_theme = alive::theme::ColorPalette::instance();
    m_ruler_pixmap.fill(color_theme->primary_background_color());
    QPainter painter(&m_ruler_pixmap);
    painter.setPen(color_theme->primary_font_color());
    painter.scale(pixel_ratio, pixel_ratio);

    const int ruler_height = height();
    int max_div_count = width() / (2 * ruler_height);

    int num_frames = m_range_end - m_range_start;
    // int num_seconds = num_frames / m_fps;

    // check if we can draw each frame upto 5 frames per division
    int num_frames_per_division = num_frames / max_div_count + 1;
    bool draw_frame_based = false;
    if (num_frames_per_division <= m_fps / 2) {
        draw_frame_based = true;
    }

    // check for seconds, if frames does not feet
    if (!draw_frame_based) {
        num_frames_per_division = m_fps;
        while (1) {
            int div_count = num_frames / num_frames_per_division;
            if (div_count <= max_div_count) {
                break;
            }

            num_frames_per_division *= 2;
        }
    }

    static const QString text("%1s%2");
    // draw seconds
    int top = ruler_height - m_ruler_tick_height / 2;
    int bottom = top + m_ruler_tick_height;
    for (int i = m_range_start; i <= m_range_end; i += num_frames_per_division) {
        int left_x = m_helper->position_for_frame(i);
        painter.drawLine(left_x, top, left_x, bottom);

        // draw text
        QString no_of_frames_text;
        if (draw_frame_based)
            no_of_frames_text = QString::number((i % static_cast<int>(m_fps)));

        QString t = text.arg(i / static_cast<int>(m_fps)).arg(no_of_frames_text);
        QFontMetrics fm(QGuiApplication::font());
        auto t_rect = QRect(0, 0, 1, fm.height());
        t_rect.setWidth(fm.horizontalAdvance(t));
        t_rect.moveCenter(QPoint(left_x, top - t_rect.height() / 2));
        painter.drawText(t_rect, t);
    }
}
} // namespace Alive
