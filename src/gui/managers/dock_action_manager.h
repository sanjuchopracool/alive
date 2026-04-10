#ifndef DOCK_ACTION_MANAGER_H
#define DOCK_ACTION_MANAGER_H

#include <memory>
#include <QObject>

namespace alive {

struct DockActionManagerPrivateData;
class DockWidget;

class DockActionManager : public QObject
{
    Q_OBJECT

public:
    ~DockActionManager();

    static void init();
    static void deinit(bool qml = false);
    static DockActionManager *instance();

    void set_current_dock(DockWidget *dock);

public:
    void play_pause();
    void redo();
    void undo();

private:
    explicit DockActionManager(QObject *parent = nullptr);

private:
    std::unique_ptr<DockActionManagerPrivateData> m_d;
};

} // namespace alive

#endif // DOCK_ACTION_MANAGER_H
