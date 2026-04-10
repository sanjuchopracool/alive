#include "toolbar.h"
#include "gui/widgets/utility.h"
#include <gui/framework/theme/color_palette.h>
#include <gui/widgets/utility.h>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>

namespace alive {

constexpr QSize k_toolbutton_size(60, 60);

ToolBar::ToolBar(Qt::Orientation direction, QWidget *parent)
    : QWidget{parent}
    , m_direction(direction)
{
    setMouseTracking(true);
    QLayout *main_layout = nullptr;
    if (m_direction == Qt::Horizontal) {
        m_layout = hbox_layout();
        auto l = hbox_layout();
        l->addLayout(m_layout);
        l->addStretch();
        main_layout = l;
    } else {
        m_layout = vbox_layout();
        auto l = vbox_layout();
        l->addLayout(m_layout);
        l->addStretch();
        main_layout = l;
    }

    setFixedWidth(k_toolbutton_size.width());
    setLayout(main_layout);
}

QSize ToolBar::sizeHint() const
{
    return k_toolbutton_size;
}

void ToolBar::add_toolbutton(ToolButton *btn)
{
    if (btn) {
        m_layout->addWidget(btn);
        connect(btn, &ToolButton::selected, this, &ToolBar::on_toolbutton_selected);
    }
}

void ToolBar::paintEvent(QPaintEvent *ev)
{
    QPainter painter(this);
    const auto *colors = theme::ColorPalette::instance();
    painter.fillRect(this->rect(), colors->dark_color());
    QWidget::paintEvent(ev);
}

void ToolBar::on_toolbutton_selected(ToolButton *btn, bool flag)
{
    if (btn) {
        if (flag) {
            if (m_selected) {
                QSignalBlocker blk(m_selected);
                m_selected->select(false);
            }
            m_selected = btn;
        } else {
            m_selected = nullptr;
        }
        emit selected(btn, flag);
    }
}

ToolButton::ToolButton(const QString &name, const QIcon &icon, QWidget *parent)
    : QWidget(parent)
    , m_name(name)
{
    setFixedSize(k_toolbutton_size);
    int size = k_toolbutton_size.width() * 0.6;
    m_normal_pixmap = icon.pixmap(size, size);

    const auto *colors = theme::ColorPalette::instance();

    {
        m_normal_pixmap = QPixmap(m_normal_pixmap);
        QPainter pixmapPainter(&m_normal_pixmap);
        pixmapPainter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
        pixmapPainter.fillRect(m_normal_pixmap.rect(), colors->primary_font_color());
        pixmapPainter.end();
    }

    {
        m_active_pixmap = QPixmap(m_normal_pixmap);
        QPainter pixmapPainter(&m_active_pixmap);
        pixmapPainter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
        pixmapPainter.fillRect(m_active_pixmap.rect(), colors->bright_color());
        pixmapPainter.end();
    }
}

void ToolButton::select(bool select)
{
    if (m_selected != select) {
        m_selected = select;
        update();
        emit selected(this, select);
    }
}

const QString &ToolButton::name() const
{
    return m_name;
}

QSize ToolButton::sizeHint() const
{
    return k_toolbutton_size;
}

void ToolButton::paintEvent(QPaintEvent *ev)
{
    QStyleOption option;
    option.initFrom(this);

    QPainter painter(this);
    const auto *colors = theme::ColorPalette::instance();
    if (m_selected) {
        painter.fillRect(this->rect(), colors->secondary_background_color());
    } else {
        painter.fillRect(this->rect(), colors->dark_color());
    }
    bool underMouse = option.state & QStyle::State_MouseOver;
    if (underMouse || m_selected) {
        painter.setPen(colors->bright_color());
    }

    int tenth_size = width() / 10;
    {
        painter.save();
        painter.translate(width() / 2, tenth_size / 2);
        QRect icon_rect(0, 0, tenth_size * 6, tenth_size * 6);
        icon_rect.translate(-icon_rect.width() / 2, 0);
        if (underMouse || m_selected) {
            painter.drawPixmap(icon_rect, m_active_pixmap);
        } else {
            painter.drawPixmap(icon_rect, m_normal_pixmap);
        }
        painter.restore();
    }

    QFont font = qApp->font();
    font.setPointSize(tenth_size * 1.5);
    QFontMetrics fm(font);
    auto text_width = fm.horizontalAdvance(m_name);
    painter.translate((width() - text_width) / 2, tenth_size * 8.5);

    painter.setFont(font);
    painter.drawText(0, 0, m_name);
}

void ToolButton::enterEvent(QEnterEvent *ev)
{
    QWidget::enterEvent(ev);
    update();
}

void ToolButton::leaveEvent(QEvent *ev)
{
    QWidget::leaveEvent(ev);
    update();
}

void ToolButton::mousePressEvent(QMouseEvent *ev)
{
    select(!m_selected);
    ev->accept();
}

} // namespace alive
