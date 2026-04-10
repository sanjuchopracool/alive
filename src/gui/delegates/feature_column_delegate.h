#ifndef FEATURE_COLUMN_DELEGATE_H
#define FEATURE_COLUMN_DELEGATE_H
#include <core/alive_types/common_types.h>

#include <QAbstractItemDelegate>
namespace alive::model {
class CompositionDocument;
}

namespace alive {

class FeatureColumnDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
public:
    explicit FeatureColumnDelegate(QAbstractItemModel *model, QObject *parent = nullptr);

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

protected:
    bool editorEvent(QEvent *event,
                     QAbstractItemModel *model,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;
signals:
    void go_to_time(alive::KeyFrameTime);

private:
    QAbstractItemModel *m_model;
    int m_height = 10;
};

} // namespace alive

#endif // FEATURE_COLUMN_DELEGATE_H
