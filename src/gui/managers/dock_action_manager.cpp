#include "dock_action_manager.h"
#include <gui/widgets/dock_widget.h>

namespace alive {

namespace {

static DockActionManager *the_instance = nullptr;
} // namespace

struct DockActionManagerPrivateData
{
    DockWidget *current_dock = nullptr;
};

DockActionManager::DockActionManager(QObject *parent)
    : QObject{parent}
    , m_d(std::make_unique<DockActionManagerPrivateData>())
{}

DockActionManager::~DockActionManager() {}

void DockActionManager::init()
{
    if (!the_instance) {
        the_instance = new DockActionManager();
    }
}

void DockActionManager::deinit(bool qml)
{
    if (!qml && the_instance) {
        delete the_instance;
        the_instance = nullptr;
    }
}

DockActionManager *DockActionManager::instance()
{
    return the_instance;
}

void DockActionManager::set_current_dock(DockWidget *dock)
{
    m_d->current_dock = dock;
}

void DockActionManager::play_pause()
{
    if (m_d->current_dock) {
        m_d->current_dock->play_pause();
    }
}

void DockActionManager::redo()
{
    if (m_d->current_dock) {
        m_d->current_dock->redo();
    }
}

void DockActionManager::undo()
{
    if (m_d->current_dock) {
        m_d->current_dock->undo();
    }
}

} // namespace alive
