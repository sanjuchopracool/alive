#ifndef DOCK_WIDGET_H
#define DOCK_WIDGET_H

#include "alive_widget.h"
#include <memory>
#include <QWidget>

namespace alive {

class DockWidgetPrivateData;
class DockWidget : public QWidget, public WidgetInterface
{
    Q_OBJECT
public:
    explicit DockWidget(QWidget *parent = nullptr);
    ~DockWidget();

    void set_focus(bool focus);

    virtual void play_pause();
    virtual void undo();
    virtual void redo();

protected:
    void paintEvent(QPaintEvent *ev) override;

private:
    std::unique_ptr<DockWidgetPrivateData> m_d;
};

} // namespace alive

#endif // DOCK_WIDGET_H
