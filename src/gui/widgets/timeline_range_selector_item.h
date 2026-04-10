#ifndef TIMELINE_RANGE_SELECTOR_ITEM_H
#define TIMELINE_RANGE_SELECTOR_ITEM_H

#include <QPainterPath>
#include <QWidget>

namespace alive::gui {
class TimelineHelper;
} // namespace alive::gui

namespace alive {
class TimelineRangeSelectorItem : public QWidget
{
    Q_OBJECT
public:
    enum SelectionType { e_VisibleRange, e_PlayRange };
    explicit TimelineRangeSelectorItem(alive::gui::TimelineHelper *helper,
                                       SelectionType type,
                                       QWidget *parent = nullptr);

    QSize sizeHint() const override;
    ~TimelineRangeSelectorItem();

signals:

protected:
    void paintEvent(QPaintEvent *ev) override;
    void resizeEvent(QResizeEvent *ev) override;
    void enterEvent(QEnterEvent *ev) override;
    void leaveEvent(QEvent *ev) override;
    void mouseMoveEvent(QMouseEvent *ev) override;
    void mousePressEvent(QMouseEvent *ev) override;
    void mouseReleaseEvent(QMouseEvent *ev) override;

private:
    enum MousePositionArea { e_None, e_LeftHandle, e_Range, e_RightHandle };
    void update_path();
    MousePositionArea find_overlap_area(qreal x) const;
    void update_cursor(MousePositionArea area);
    bool is_dragging() const { return m_drag_area != e_None; }

private:
    QPainterPath m_handle_path;

    alive::gui::TimelineHelper *m_helper = nullptr;
    SelectionType m_type = e_VisibleRange;
    MousePositionArea m_mouse_pos_area = e_None;
    MousePositionArea m_drag_area = e_None;
    int m_cursor_id = 0;
};
} // namespace Alive

#endif // TIMELINE_RANGE_SELECTOR_ITEM_H
