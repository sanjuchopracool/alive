#ifndef TIMELINE_COLUMN_DELEGATE_H
#define TIMELINE_COLUMN_DELEGATE_H

#include <QAbstractItemDelegate>

namespace alive::model {
class CompositionDocument;
}

namespace alive::gui {
class TimelineHelper;
}

namespace alive {

class TimelineColumnDelegate : public QAbstractItemDelegate
{
public:
    explicit TimelineColumnDelegate(QAbstractItemModel *model,
                                    const alive::gui::TimelineHelper *helper,
                                    QObject *parent = nullptr);

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    bool editorEvent(QEvent *event,
                     QAbstractItemModel *model,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;

private:
    QAbstractItemModel *m_model;
    const alive::gui::TimelineHelper *m_helper;
    int m_height = 10;
};

} // namespace alive

#endif // TIMELINE_COLUMN_DELEGATE_H
