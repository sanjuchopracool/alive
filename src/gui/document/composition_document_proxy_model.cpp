#include "composition_document_proxy_model.h"
#include "composition_document_model.h"

namespace alive {
CompositionDocumentProxyModel::CompositionDocumentProxyModel(model::CompositionDocumentModel *source)
    : QSortFilterProxyModel(source)
    , m_source(source)
{
    setSourceModel(source);
}

void CompositionDocumentProxyModel::filter(const QString &filter)
{
    setFilterWildcard(filter);
}

bool CompositionDocumentProxyModel::filterAcceptsRow(int source_row,
                                                     const QModelIndex &source_parent) const
{
    // Implement only layer filter for now
    if (!source_parent.isValid()) {
        auto index = sourceModel()->index(source_row, 0, source_parent);
        return sourceModel()->data(index).toString().contains(filterRegularExpression());
    }

    return true;
}

} // namespace alive
