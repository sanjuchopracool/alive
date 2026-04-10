#include "cursor_manager.h"
#include <QCursor>
#include <QGuiApplication>
#include <QIcon>

namespace alive::gui {

struct CursorManagerData
{
    CursorManagerData()
    {
        left_drag_cursor = QCursor(QIcon::fromTheme("left_drag_cursor").pixmap(QSize(16, 16)));
        right_drag_cursor = QCursor(QIcon::fromTheme("right_drag_cursor").pixmap(QSize(16, 16)));
        hor_size_cursor = QCursor(QIcon::fromTheme("hor_size_cursor").pixmap(QSize(16, 16)));
    }

    bool overridden = false;
    QCursor left_drag_cursor;
    QCursor right_drag_cursor;
    QCursor hor_size_cursor;

    bool is_left_drag_cursor() { return qGuiApp->overrideCursor() == &left_drag_cursor; }
    bool is_right_drag_cursor() { return qGuiApp->overrideCursor() == &right_drag_cursor; }
    bool is_hor_size_cursor() { return qGuiApp->overrideCursor() == &hor_size_cursor; }

    void restore_override_cursor()
    {
        if (overridden) {
            qGuiApp->restoreOverrideCursor();
            overridden = false;
        }
    }

    int counter = 0;
};

CursorManager::~CursorManager() {}

int CursorManager::horizontal_size_cursor()
{
    m_d->counter++;
    m_d->restore_override_cursor();
    qGuiApp->setOverrideCursor(m_d->hor_size_cursor);
    m_d->overridden = true;
    return m_d->counter;
}

int CursorManager::left_drag_cursor()
{
    m_d->counter++;
    m_d->restore_override_cursor();
    qGuiApp->setOverrideCursor(m_d->left_drag_cursor);
    m_d->overridden = true;
    return m_d->counter;
}

int CursorManager::right_drag_cursor()
{
    m_d->counter++;
    m_d->restore_override_cursor();
    qGuiApp->setOverrideCursor(m_d->right_drag_cursor);
    m_d->overridden = true;
    return m_d->counter;
}

void CursorManager::restore_cursor(int id)
{
    if (m_d->overridden && m_d->counter == id) {
        qGuiApp->restoreOverrideCursor();
        m_d->overridden = false;
    }
}

CursorManager::CursorManager(QObject *parent)
    : QObject{parent}
    , m_d(std::make_unique<CursorManagerData>())
{

}
} // namespace alive::gui
