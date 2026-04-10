#ifndef COMPOSITIONDOCUMENTMODEL_H
#define COMPOSITIONDOCUMENTMODEL_H

#include "composition_document.h"
#include "gui/document/composition_document_proxy_model.h"

namespace alive {
class CompositionDocumentProxyModel;
}

namespace alive::model {
class CompositionDocumentModel : public CompositionDocument
{
    Q_OBJECT
public:
    CompositionDocumentModel(Composition *comp, QObject *parent = nullptr);

    QVariant get_keyframe_status(const QModelIndex &index, KeyFrameTime time) const;
    void removed_keyframe_at(const QModelIndex &index, KeyFrameTime time);
    void add_keyframe_at(const QModelIndex &index, KeyFrameTime time);
    void remove_all_keyframes(const QModelIndex &index);
    void invoke_context_menu_for_property(const QModelIndex &index);

    Q_INVOKABLE alive::CompositionDocumentProxyModel *proxy_model() override;
    Q_INVOKABLE void drag_and_drop(TreeItem *from, TreeItem *to, bool above);

private:
    alive::CompositionDocumentProxyModel *m_proxy_model = nullptr;
};
} // namespace alive::model

#endif // COMPOSITIONDOCUMENTMODEL_H
