#ifndef COMPOSITIONDOCUMENTPROXYMODEL_H
#define COMPOSITIONDOCUMENTPROXYMODEL_H

#include <core/alive_types/common_types.h>
#include <QSortFilterProxyModel>

namespace alive {

namespace model {
class CompositionDocumentModel;
}

class CompositionDocumentProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    CompositionDocumentProxyModel(model::CompositionDocumentModel *source);
    void filter(const QString &filter);
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    model::CompositionDocumentModel *m_source = nullptr;
};

} // namespace alive
#endif // COMPOSITIONDOCUMENTPROXYMODEL_H
