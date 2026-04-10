#include "timeline_ruler.h"
#include <QFont>
#include <QFontMetrics>
#include <QGuiApplication>
#include <QPainter>
#include <QPainterPath>
#include <QQuickWindow>

namespace alive::gui {
TimelineRuler::TimelineRuler(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
    connect(this, &QQuickPaintedItem::widthChanged, this, &TimelineRuler::update_pixmap);
    connect(this, &QQuickPaintedItem::heightChanged, this, &TimelineRuler::update_pixmap);
}

void TimelineRuler::paint(QPainter *painter)
{
    painter->save();
    painter->drawPixmap(boundingRect().toRect(), m_ruler_pixmap);
    painter->restore();
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

void TimelineRuler::update_pixmap()
{
    if (!width())
        return;

    //    qDebug() << "Ruler Width: " << width();
    qreal effective_width = width() - 2 * m_handle_width;
    qreal pixel_ratio = window()->effectiveDevicePixelRatio();
    m_ruler_pixmap = QPixmap(size().toSize() * pixel_ratio);
    //    qDebug() << "Ruler Pixmap Width: " << m_ruler_pixmap.size().width();
    m_ruler_pixmap.fill(Qt::transparent);
    QPainter painter(&m_ruler_pixmap);
    painter.scale(pixel_ratio, pixel_ratio);

    const int ruler_height = height();
    int max_div_count = width() / (2 * ruler_height);

    int num_frames = m_range_end - m_range_start;

    // check if we can draw each frame upto 5 frames per division
    int num_frames_per_division = 1;
    bool draw_frame_based = false;
    while (num_frames_per_division <= 5) {
        int div_count = num_frames / num_frames_per_division;
        if (div_count <= max_div_count) {
            draw_frame_based = true;
            break;
        }

        num_frames_per_division++;
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

    static QString text("%1s%2");
    // draw seconds
    qreal top = ruler_height - m_ruler_tick_height / 2;
    qreal bottom = top + m_ruler_tick_height;
    int range = m_range_end - m_range_start;
    painter.setPen(m_text_color);
    for (int i = m_range_start; i <= m_range_end; i += num_frames_per_division) {
        qreal left_x = (((i - m_range_start) * effective_width) / range) + m_handle_width;
        //        qDebug() << i << left_x;
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
} // namespace alive::gui
