#include "composition_document_model.h"
#include "composition_document_proxy_model.h"

namespace alive::model {
CompositionDocumentModel::CompositionDocumentModel(Composition *comp, QObject *parent)
    : CompositionDocument(comp, parent)
{
    m_proxy_model = new alive::CompositionDocumentProxyModel(this);
}

QVariant CompositionDocumentModel::get_keyframe_status(const QModelIndex &index,
                                                       KeyFrameTime time) const
{
    const TreeItem *item = item_from_index(index);
    if (item) {
        return get_property_keyframe_status(item->to_property(), time);
    }

    return {};
}

void CompositionDocumentModel::removed_keyframe_at(const QModelIndex &index, KeyFrameTime time)
{
    TreeItem *item = item_from_index(index);
    if (item) {
        property_remove_keyframe_at(item->to_property(), time);
    }
}

void CompositionDocumentModel::add_keyframe_at(const QModelIndex &index, KeyFrameTime time)
{
    TreeItem *item = item_from_index(index);
    if (item) {
        property_add_keyframe_at(item->to_property(), time);
    }
}

void CompositionDocumentModel::remove_all_keyframes(const QModelIndex &index)
{
    TreeItem *item = item_from_index(index);
    if (item) {
        property_remove_all_keyframes(item->to_property());
    }
}

void CompositionDocumentModel::invoke_context_menu_for_property(const QModelIndex &index)
{
    TreeItem *item = item_from_index(index);
    if (item) {
        CompositionDocument::invoke_context_menu_for_property(item);
    }
}

CompositionDocumentProxyModel *CompositionDocumentModel::proxy_model()
{
    return m_proxy_model;
}

void CompositionDocumentModel::drag_and_drop(TreeItem *from, TreeItem *to, bool above)
{
    move_items(from, to, above);
}
} // namespace alive::model
