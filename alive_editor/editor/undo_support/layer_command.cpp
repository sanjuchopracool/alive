#include <editor/undo_support/layer_command.h>
#include <editor/undo_support/command_enums.h>
#include <editor/document/document.h>
#include <editor/undo_support/document_undo_manager.h>

namespace alive {

AddRemoveLayerCommand::AddRemoveLayerCommand(const std::vector<model::Layer *> &layers,
                                             Type type,
                                             int index)
    : LayerCommand(nullptr, CommandType::e_AddRemoveNewLayer)
    , m_layers(layers)
    , m_indices(m_layers.size(), -1)
    , m_type(type)
    , m_index(index)
{
}

AddRemoveLayerCommand::~AddRemoveLayerCommand()
{
    if (m_deleted) {
        // qDeleteAll(m_layers);
    }
}

void AddRemoveLayerCommand::redo()
{
    if (m_type == e_Add) {
        add_layers();
    } else {
        remove_layers();
    }
}

void AddRemoveLayerCommand::undo()
{
    if (m_type == e_Add) {
        remove_layers();
    } else {
        add_layers();
    }
}

void AddRemoveLayerCommand::add_layers()
{
    if (m_document) {
        if (m_type == e_Add) {
            int index = m_index != -1 ? m_index : 0;
            for (int i = 0; i < m_layers.size(); ++i) {
                m_indices[i] = m_document->undo_manager()->add_layer(m_layers[i], index + i);
            }
        } else {
            for (int i = 0; i < m_layers.size(); ++i) {
                m_document->undo_manager()->add_layer(m_layers[i], m_indices[i]);
            }
        }
        m_deleted = false;
    }
}

void AddRemoveLayerCommand::remove_layers()
{
    if (m_document) {
        for (int i = 0; i < m_layers.size(); ++i) {
            m_indices[i] = m_document->undo_manager()->remove_layer(m_layers[i]);
        }
        m_deleted = true;
    }
}

} // namespace alive
