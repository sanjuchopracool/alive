#ifndef CANVASWIDGET_H
#define CANVASWIDGET_H

#include "alive_widget.h"
#include <core/model/property/property_update_listener.h>
#include <core/model/property/update_listener.h>
#include <gui/composition/timeline.h>
#include <gui/framework/canvas/canvas_interface.h>
#include <QUuid>
#include <QWidget>

namespace alive::model {
class CompositionDocument;
class CompositionNode;
} // namespace alive::model
namespace alive {
using namespace alive;

class CanvasWidget : public QWidget,
                     public WidgetInterface,
                     public alive::model::SimpleObserver,
                     public canvas::CanvasInterface
{
    Q_OBJECT

public:
    CanvasWidget(model::CompositionDocument *document, QWidget *parent = nullptr);
    ~CanvasWidget();

    int current_frame() const { return m_current_frame; }

    QUuid id() const;
    QString name() const;

protected:
    void paintEvent(QPaintEvent* ev) override;
    void resizeEvent(QResizeEvent *ev) override;
    void on_update(model::ISubject *) override;

signals:
    void current_frame_changed();
    void document_changed();
    void updated_nodes();
public slots:
    void slot_frame_changed(int frame, bool only_render = false, bool update_and_render = false);

private:
    void on_buffer_size_update() override;
    void resize_window(const QSize &size, bool forced = false);

protected:
    //    void focusInEvent(QFocusEvent *ev) override;
    //    void focusOutEvent(QFocusEvent *ev) override;

    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    bool m_forced_update = false;
    int m_current_frame = 0;
    QSize m_previous_size;
    bool m_updating = false;
    bool m_update_node = false;
    QBrush m_checkboard_brush;
};
} // namespace alive::gui

#endif // CANVASWIDGET_H
