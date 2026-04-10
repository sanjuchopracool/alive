#include "edit_column_header.h"
#include <gui/icon_manager.h>
#include <gui/framework/theme/color_palette.h>
#include <gui/theme/timeline_style.h>
#include <QMouseEvent>
#include <QPainter>

namespace alive {

EditColumnHeader::EditColumnHeader(QWidget *parent)
    : QWidget{parent}
{
    const auto *timeline_style = alive::theme::TimelineSize::instance();
    setFixedHeight(timeline_style->item_height());
    setMinimumWidth(alive::theme::TimelineSize::instance()->column_width());
}

void EditColumnHeader::paintEvent(QPaintEvent *ev)
{
    const IconManager *icon_mgr = IconManager::instance();
    QPainter painter(this);
    const QTextOption to(Qt::AlignHCenter | Qt::AlignVCenter);
    const auto *sizes = alive::theme::TimelineSize::instance();
    if (m_show_matte_details) {
        QRect rect(this->rect());
        // const auto *colors = alive::theme::ColorPalette::instance();

        rect.setWidth(sizes->column_width());
        // blend
        painter.drawText(rect, tr("Blend"), to);
        // matte
        rect.moveLeft(rect.right());
        painter.drawText(rect, tr("Matte"), to);
        // matte layer
        rect.moveLeft(rect.right());
        painter.drawText(rect, tr("Matte Layer"), to);
        // parent layer
        rect.moveLeft(rect.right());
        painter.drawText(rect, tr("Parent"), to);
    } else {
        painter.drawText(this->rect(), tr("Edit"), to);
    }

    QRect rect(this->rect());
    QRect icon_rect(rect.right() - sizes->icon_size().width(),
                    (rect.height() - sizes->icon_size().height()) / 2,
                    sizes->icon_size().width(),
                    sizes->icon_size().height());
    icon_mgr->get_icon(IconManager::e_Grid)
        .paint(&painter, icon_rect, Qt::AlignCenter, QIcon::Selected);
}

void EditColumnHeader::mousePressEvent(QMouseEvent *ev)
{
    const auto *sizes = alive::theme::TimelineSize::instance();
    QRect rect(this->rect());
    const QRect icon_rect(rect.right() - sizes->icon_size().width(),
                          (rect.height() - sizes->icon_size().height()) / 2,
                          sizes->icon_size().width(),
                          sizes->icon_size().height());

    if (icon_rect.contains(ev->pos())) {
        set_show_matte_info(!m_show_matte_details);
        update();
    }

    QWidget::mousePressEvent(ev);
}

void EditColumnHeader::set_show_matte_info(bool show)
{
    if (m_show_matte_details != show) {
        m_show_matte_details = show;
        int delta = 0;
        if (show) {
            m_old_width = width();
            delta = expanded_width() - m_old_width;
        } else {
            delta = m_old_width - width();
        }
        emit show_matte_info_changed(m_show_matte_details, delta);
    }
}

bool EditColumnHeader::show_matte_info() const
{
    return m_show_matte_details;
}

int EditColumnHeader::before_expand_width() const
{
    return m_old_width;
}

int EditColumnHeader::expanded_width() const
{
    const auto *sizes = alive::theme::TimelineSize::instance();
    return 4 * sizes->column_width();
}

} // namespace alive
