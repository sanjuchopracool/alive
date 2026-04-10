#ifndef COMPOSITION_ITEM_MODEL_H
#define COMPOSITION_ITEM_MODEL_H

#include <QAbstractItemModel>
#include <QObject>
#include <editor/document/document.h>

namespace alive {
class TreeItem;
}

namespace alive::model {
class Composition;
class Layer;
class CompositionDocument;
class CompositionItemModel : public QAbstractItemModel, public Document
{
    Q_OBJECT
public:
    enum Section { e_Features, e_Tree, e_Layer, e_Edit, e_TimeLine, e_SectionCount };
    Q_ENUM(Section)

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;

    Qt::DropActions supportedDropActions() const override;

protected:
    QModelIndex index_of(const TreeItem *item);
    TreeItem *item_from_index(const QModelIndex &index);
    const TreeItem *item_from_index(const QModelIndex &index) const;

    explicit CompositionItemModel(Composition *comp, QObject *parent = nullptr);
};
} // namespace alive::model

#endif // COMPOSITION_ITEM_MODEL_H
