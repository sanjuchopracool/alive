#ifndef CURSORMANAGER_H
#define CURSORMANAGER_H

#include <memory>
#include <QObject>

QT_BEGIN_NAMESPACE
class QQmlEngine;
class QJSEngine;
QT_END_NAMESPACE

namespace alive::gui {
struct CursorManagerData;
class CursorManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(CursorManager)

public:
    ~CursorManager();
    static QObject *qml_instance(QQmlEngine *engine, QJSEngine *script_engine)
    {
        Q_UNUSED(engine);
        Q_UNUSED(script_engine);
        return instance();
    }

    static CursorManager *instance()
    {
        static CursorManager *the_instance = nullptr;
        if (!the_instance) {
            the_instance = new CursorManager();
        }
        return the_instance;
    }

    Q_INVOKABLE int horizontal_size_cursor();
    Q_INVOKABLE int left_drag_cursor();
    Q_INVOKABLE int right_drag_cursor();
    Q_INVOKABLE void restore_cursor(int id);

private:
    explicit CursorManager(QObject *parent = nullptr);

signals:

private:
    std::unique_ptr<CursorManagerData> m_d;
};
} // namespace alive::gui

#endif // CURSORMANAGER_H
