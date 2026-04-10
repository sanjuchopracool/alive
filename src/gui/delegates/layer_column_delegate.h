#ifndef LAYER_COLUMN_DELEGATE_H
#define LAYER_COLUMN_DELEGATE_H

#include <QAbstractItemDelegate>

namespace alive::model {
class CompositionDocument;
}

namespace alive {

class LayerColumnDelegate : public QAbstractItemDelegate
{
public:
    explicit LayerColumnDelegate(QAbstractItemModel *model, QObject *parent = nullptr);

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

protected:
    bool editorEvent(QEvent *event,
                     QAbstractItemModel *model,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;

private:
    QAbstractItemModel *m_model;
    int m_height = 10;
};

} // namespace alive

#endif // LAYER_COLUMN_DELEGATE_H
