#include <core/model/shape_items/shape_item.h>
#include <editor/document/document.h>
#include <editor/undo_support/command_enums.h>
#include <editor/undo_support/document_undo_manager.h>
#include <editor/undo_support/shape_item_commands.h>

namespace alive {

void ShapeItemCommand::update_document()
{
    if (m_document) {
        m_document->undo_manager()->updated_item(m_shape, m_roles);
    }
}

SetPathShapeDirectionCommand::SetPathShapeDirectionCommand(model::PathShapeItem *shape,
                                                           PathDirection new_direction)
    : ShapeItemCommand(shape, CommandType::e_SetPathShapeDirection)
    , m_path_shape(shape)
    , m_new_direction(new_direction)
{}

void SetPathShapeDirectionCommand::redo()
{
    m_old_direction = m_path_shape->path_direction();
    m_path_shape->set_path_direction(m_new_direction);
    update_document();
}

void SetPathShapeDirectionCommand::undo()
{
    m_path_shape->set_path_direction(m_old_direction);
    update_document();
}

} // namespace alive
