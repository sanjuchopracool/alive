#include "composition_item_model.h"
#include <core/model/composition.h>
#include <gui/theme/layer_color_model.h>
#include <QIcon>
namespace alive::model {

CompositionItemModel::CompositionItemModel(Composition *comp, QObject *parent)
    : QAbstractItemModel{parent}
    , Document(comp)
{}

Qt::ItemFlags CompositionItemModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = Qt::ItemIsEnabled;
    auto column = static_cast<Section>(index.column());
    if (column == e_Edit) {
        flags |= Qt::ItemIsEditable;
    } else if (column == e_Tree) {
        flags |= Qt::ItemIsSelectable;
        const TreeItem *item = item_from_index(index);
        if (item) {
            const model::Object *obj = item->to_object();
            if (obj) {
                if ((obj->is_layer() || obj->is_shape_item())) {
                    flags |= Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
                }

                if (obj->can_rename()) {
                    flags |= Qt::ItemIsEditable;
                }
            }
        }
    } else if (column == e_TimeLine) {
        flags |= Qt::ItemIsSelectable;
    }
    return flags;
}

int CompositionItemModel::rowCount(const QModelIndex &parent) const
{
    int row_count = 0;
    if (!parent.isValid()) {
        row_count = m_comp->item_count();
    } else {
        const TreeItem *item = static_cast<const TreeItem *>(parent.internalPointer());
        return item->item_count();
    }

    return row_count;
}

int CompositionItemModel::columnCount(const QModelIndex &parent) const
{
    return static_cast<int>(Section::e_SectionCount);
}

QModelIndex CompositionItemModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        if (row < m_comp->item_count())
            return createIndex(row, column, m_comp->item_at(row));
    } else {
        const TreeItem *parent_object = static_cast<const TreeItem *>(parent.internalPointer());
        if (row < parent_object->item_count())
            return createIndex(row, column, parent_object->item_at(row));
    }

    return QModelIndex();
}

QModelIndex CompositionItemModel::parent(const QModelIndex &child) const
{
    if (child.internalPointer()) {
        const TreeItem *item = static_cast<const TreeItem *>(child.internalPointer());
        const TreeItem *parent = item->parent();
        if (parent) {
            const TreeItem *grand_parent = parent->parent();
            if (grand_parent) {
                return createIndex(grand_parent->index_of_item(parent), 0, parent);
            }
        }
    }

    return QModelIndex();
}

Qt::DropActions CompositionItemModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

QModelIndex CompositionItemModel::index_of(const TreeItem *item)
{
    if (item) {
        const TreeItem *parent = item->parent();
        if (parent) {
            int index = parent->index_of_item(item);
            return createIndex(index, 0, item);
        }
    }
    return QModelIndex();
}

TreeItem *CompositionItemModel::item_from_index(const QModelIndex &index)
{
    return static_cast<TreeItem *>(index.internalPointer());
}

const TreeItem *CompositionItemModel::item_from_index(const QModelIndex &index) const
{
    return static_cast<const TreeItem *>(index.internalPointer());
}

} // namespace alive::model
