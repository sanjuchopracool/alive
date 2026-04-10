#include "undostack_model.h"

namespace alive::model {

UndoStackModel::UndoStackModel(const QUndoStack *stack, QObject *parent)
    : QAbstractListModel{parent}
    , m_stack(stack)
{
    
}

int UndoStackModel::rowCount(const QModelIndex &parent) const
{
    return m_stack->count();
}

QVariant UndoStackModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        return m_stack->text(index.row());
    }
    return {};
}

void UndoStackModel::reset_data()
{
    beginResetModel();
    endResetModel();
}
} // namespace alive::model
