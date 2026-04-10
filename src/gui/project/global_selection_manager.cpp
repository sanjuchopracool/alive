#include "global_selection_manager.h"
#include <core/model/composition.h>
#include <core/model/layers/precomposition_layer.h>
#include <core/model/layers/text_layer.h>
#include <gui/document/composition_document.h>
#include <gui/framework/widgets/toolbar_property_widget.h>
#include <gui/project/project_manager.h>
#include <map>

namespace alive {

class TagModel;
struct SelectionContext
{
    model::TextLayer *text_layer = nullptr;
    model::PrecompositionLayer *comp_layer = nullptr;
    model::PropertyGroup *prop_group = nullptr;
    TagModel *tag_model = nullptr;

    void reset()
    {
        text_layer = nullptr;
        comp_layer = nullptr;
        prop_group = nullptr;
        tag_model = nullptr;
    }
};

struct GlobalSelectionManagerPrivateData
{
    model::CompositionDocument *document = nullptr;
    ToolBarPropertyWidget *toolbar = nullptr;

    // selection Context
    std::map<model::CompositionDocument *, SelectionContext> contexts;
};

GlobalSelectionManager::GlobalSelectionManager(ToolBarPropertyWidget *toolbar, QObject *parent)
    : QObject(parent)
    , m_d(Corrade::Containers::pointer<GlobalSelectionManagerPrivateData>())
{
    m_d->toolbar = toolbar;
    auto *prj_mgr = alive::project::ProjectManager::instance();
    connect(prj_mgr, &alive::project::ProjectManager::current_doc_changed, this, [=, this]() {
        alive::model::CompositionDocument *doc = prj_mgr->current_document();
        m_d->document = doc;
        if (doc) {
            connect(doc,
                    &model::CompositionDocument::text_layer_settings_changed,
                    this,
                    &GlobalSelectionManager::text_layer_settings_changed,
                    Qt::UniqueConnection);

            auto it = m_d->contexts.find(doc);
            if (it != m_d->contexts.end()) {
            } else {
                m_d->contexts[doc] = SelectionContext();
                m_d->contexts[doc].tag_model = doc->real_tag_model();
                m_d->contexts[doc].prop_group = doc->composition()->property_group();
            }
        }

        update_toolbars();
    });

    connect(prj_mgr,
            &alive::project::ProjectManager::about_to_close_document,
            this,
            [=, this](model::CompositionDocument *doc) {
                if (doc) {
                    m_d->contexts.erase(doc);
                    if (m_d->document == doc) {
                        m_d->document = nullptr;
                    }
                }
                update_toolbars();
            });

    connect(m_d->toolbar,
            &ToolBarPropertyWidget::text_changed,
            this,
            [this](const core::Text &text) {
                if (m_d->document) {
                    auto it = m_d->contexts.find(m_d->document);
                    if (it != m_d->contexts.end() && it->second.text_layer) {
                        m_d->document->update_text_layer_settings(it->second.text_layer, text);
                    }
                }
            });
}

GlobalSelectionManager::~GlobalSelectionManager() {}

void GlobalSelectionManager::on_layer_selected(model::Layer *layer)
{
    auto it = m_d->contexts.find(m_d->document);
    if (it != m_d->contexts.end()) {
        auto &context = it->second;
        context.reset();
        if (layer) {
            context.text_layer = layer->to_text_layer();
            context.comp_layer = layer->to_precomp_layer();
            if (context.comp_layer) {
                context.prop_group = context.comp_layer->property_group();
            }
        } else {
            context.prop_group = m_d->document->composition()->property_group();
            context.tag_model = m_d->document->real_tag_model();
        }
    }
    update_toolbars();
}

void GlobalSelectionManager::text_layer_settings_changed(model::TextLayer *text_layer,
                                                         const core::Text &text)
{
    if (QObject::sender() == m_d->document) {
        auto it = m_d->contexts.find(m_d->document);
        if (it != m_d->contexts.end() && it->second.text_layer
            && it->second.text_layer == text_layer) {
            m_d->toolbar->set_text_properties(text);
        }
    }
}

void GlobalSelectionManager::update_toolbars()
{
    SelectionContext *context = nullptr;
    if (m_d->document) {
        auto it = m_d->contexts.find(m_d->document);
        if (it != m_d->contexts.end()) {
            context = &it->second;
        }
    }

    bool show_null = !context;
    if (context) {
        // Text
        if (context->text_layer) {
            m_d->toolbar->set_text_properties(context->text_layer->get_text());
        }
        m_d->toolbar->enable_text(context->text_layer);

        // Property Group
        m_d->toolbar->set_property_group(m_d->document, context->prop_group, !context->comp_layer);

        // tagging
        m_d->toolbar->set_tag_model(context->tag_model);

        if (!context->text_layer && !context->prop_group && !context->tag_model) {
            show_null = true;
        }
    }

    m_d->toolbar->update_current_widget();
}

} // namespace alive
