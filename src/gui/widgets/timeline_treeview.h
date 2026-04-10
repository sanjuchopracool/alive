#ifndef TIMELINE_TREEVIEW_H
#define TIMELINE_TREEVIEW_H

#include <QTreeView>
#include <QWidget>

namespace alive::model {
class CompositionDocument;
class Layer;
}

namespace alive::gui {
class TimelineHelper;
}

namespace alive {
class TreeItem;
class TimelineRangeSelector;
struct TimelineTreeViewPrivateData;

void init_timeline_globals();

class TimelineTreeView : public QTreeView
{
    Q_OBJECT
public:
    TimelineTreeView(alive::model::CompositionDocument *doc,
                     const TimelineRangeSelector *range_selector,
                     QWidget *parent = nullptr);
    ~TimelineTreeView();
    void adjust_after_column_resize(int layer_column_width);

signals:
    void show_layer_menu(alive::model::Layer *layer, QPoint pos);
    void layer_selected(alive::model::Layer *layer);

public slots:
    void set_expression();
    void disable_expression();
    void show_expression_editor(bool show);

protected:
    // void paintEvent(QPaintEvent *ev) override;
    void keyPressEvent(QKeyEvent *ev) override;

    void enterEvent(QEnterEvent *ev) override;
    void leaveEvent(QEvent *ev) override;
    void mouseMoveEvent(QMouseEvent *ev) override;

    void mousePressEvent(QMouseEvent *ev) override;
    void mouseReleaseEvent(QMouseEvent *ev) override;
    void mouseDoubleClickEvent(QMouseEvent *ev) override;

    // drag events
    void startDrag(Qt::DropActions supported_actions) override;
    void dragEnterEvent(QDragEnterEvent *ev) override;
    void dragLeaveEvent(QDragLeaveEvent *ev) override;
    void dragMoveEvent(QDragMoveEvent *ev) override;
    void dropEvent(QDropEvent *ev) override;

    // row
    void drawRow(QPainter *painter,
                 const QStyleOptionViewItem &option,
                 const QModelIndex &index) const override;

    void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const override;

private slots:
    void slot_selection_changed();

private:
    void update_cursor(QMouseEvent *ev);
    void update_expression_editor_geometry();
    std::pair<std::map<alive::TreeItem *, int>, std::vector<model::Layer *>> selected_items();

private:
    std::unique_ptr<TimelineTreeViewPrivateData> m_d;
};

} // namespace alive

#endif // TIMELINE_TREEVIEW_H
