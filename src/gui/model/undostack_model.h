#ifndef UNDOSTACKMODEL_H
#define UNDOSTACKMODEL_H

#include <QAbstractListModel>
#include <QObject>
#include <QUndoStack>

namespace alive::model {

class UndoStackModel : public QAbstractListModel
{
public:
    explicit UndoStackModel(const QUndoStack *stack, QObject *parent = nullptr);

    // QAbstractListModel
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    void reset_data();

private:
    const QUndoStack *m_stack;
};
} // namespace alive::model

#endif // UNDOSTACKMODEL_H
