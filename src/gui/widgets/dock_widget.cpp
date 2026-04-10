#include "dock_widget.h"
#include <gui/project/project_manager.h>
#include <gui/framework/theme/color_palette.h>
#include <QIcon>
#include <QPainter>
#include <QStyleOption>

namespace alive {

struct DockWidgetPrivateData
{
    bool focused = false;
};

DockWidget::DockWidget(QWidget *parent)
    : QWidget{parent}
    , m_d(std::make_unique<DockWidgetPrivateData>())
{
    setFocusPolicy(Qt::ClickFocus);
}

DockWidget::~DockWidget() {}

void DockWidget::set_focus(bool focus)
{
    if (m_d->focused != focus) {
        m_d->focused = focus;
        QWidget::update();
    }
}

void DockWidget::play_pause()
{
    alive::project::ProjectManager::instance()->toggle_timeline();
}

void DockWidget::undo()
{
    alive::project::ProjectManager::instance()->undo();
}

void DockWidget::redo()
{
    alive::project::ProjectManager::instance()->redo();
}

void DockWidget::paintEvent(QPaintEvent *ev)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    if (m_d->focused) {
        p.setPen(QPen(alive::theme::ColorPalette::instance()->accent_color(), 1));
        QRectF rect(this->rect());
        p.drawRect(rect.adjusted(0.5, 0.5, -0.5, -0.5));
    }
}
} // namespace alive
