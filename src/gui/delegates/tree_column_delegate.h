#ifndef TREE_COLUMN_DELEGATE_H
#define TREE_COLUMN_DELEGATE_H

#include <QAbstractItemDelegate>

namespace alive {
class TreeItem;
namespace model {
class CompositionDocument;
}
} // namespace alive

namespace alive {

class TreeColumnDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
public:
    explicit TreeColumnDelegate(QAbstractItemModel *model, QObject *parent = nullptr);

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    void set_layer_column_widht(int width);

protected:
    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

    void setModelData(QWidget *editor,
                      QAbstractItemModel *model,
                      const QModelIndex &index) const override;
    bool eventFilter(QObject *watched, QEvent *event) override;

protected:
    bool editorEvent(QEvent *event,
                     QAbstractItemModel *model,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;

private:
    QAbstractItemModel *m_model;
    int m_height = 10;
    int m_layer_column_width = 0;
    mutable QLineEdit *m_editor = nullptr;
};

} // namespace alive

#endif // TREE_COLUMN_DELEGATE_H
