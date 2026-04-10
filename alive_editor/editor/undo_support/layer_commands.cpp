#include <core/model/layers/shape_layer.h>
#include <editor/document/document.h>
#include <editor/undo_support/command_enums.h>
#include <editor/undo_support/document_undo_manager.h>
#include <editor/undo_support/layer_commands.h>
// #include <gui/model/layers/layer_setting.h>
namespace alive {

DragLayerRangeCommand::DragLayerRangeCommand(Type type,
                                             model::Layer *layer,
                                             FrameTimeType t,
                                             const std::string &text)
    : LayerCommand(layer, CommandType::e_LayerDragRange, text)
    , m_type(type)
    , m_delta_frame(t)
{}

void DragLayerRangeCommand::redo()
{
    apply_change(m_delta_frame);
}

void DragLayerRangeCommand::undo()
{
    apply_change(-m_delta_frame);
}

bool DragLayerRangeCommand::merge_with(const Command *other)
{
    const DragLayerRangeCommand *other_command = static_cast<const DragLayerRangeCommand *>(other);
    if (m_layer == other_command->layer() && m_type == other_command->type()) {
        m_delta_frame += other_command->delta_time();
        return true;
    }

    return false;
}

void DragLayerRangeCommand::apply_change(FrameTimeType delta)
{
    switch (m_type) {
    case e_Start:
        m_layer->set_in_point(m_layer->in_point() + delta);
        break;
    case e_End:
        m_layer->set_out_point(m_layer->out_point() + delta);
        break;
    case e_Range:
        m_layer->move_in_out_point(delta);
        break;
    default:
        break;
    }

    if (m_document) {
        m_document->undo_manager()->updated_item(m_layer);
    }
}

SetLayerBooleanCommand::SetLayerBooleanCommand(Type type,
                                               bool flag,
                                               model::Layer *layer,
                                               const std::string &text)
    : LayerCommand(layer, CommandType::e_LayerSetBool, text)
    , m_type(type)
    , m_flag(flag)
{}

void SetLayerBooleanCommand::redo()
{
    apply_change(m_flag);
}

void SetLayerBooleanCommand::undo()
{
    apply_change((!m_flag));
}

void SetLayerBooleanCommand::apply_change(bool flag)
{
    switch (m_type) {
    case e_Visible:
        m_layer->set_visible(flag);
        break;
    case e_Lock:
        m_layer->set_lock(flag);
        break;
    default:
        break;
    }
    if (m_document) {
        m_document->undo_manager()->updated_item(m_layer);
    }
}
//////////////////////////////////////////////////////////////////////////////////////////

SetLayerIntCommand::SetLayerIntCommand(Type type,
                                       int value,
                                       model::Layer *layer,
                                       const std::string &text)
    : LayerCommand(layer, CommandType::e_LayerSetInt, text)
    , m_type(type)
    , m_value(value)
{
    switch (m_type) {
    case e_ColorIndex:
        m_old_value = m_layer->editor_color_index();
        break;
    default:
        break;
    }
}

void SetLayerIntCommand::redo()
{
    apply_change(m_value);
}

void SetLayerIntCommand::undo()
{
    apply_change(m_old_value);
}

bool SetLayerIntCommand::merge_with(const Command *other)
{
    const SetLayerIntCommand *other_command = static_cast<const SetLayerIntCommand *>(other);
    if (m_layer == other_command->layer() && m_type == other_command->type()) {
        m_value = other_command->value();
        return true;
    }
    return false;
}

void SetLayerIntCommand::apply_change(int value)
{
    switch (m_type) {
    case e_ColorIndex:
        m_layer->set_editor_color_index(value);
        break;
    default:
        break;
    }
    if (m_document) {
        m_document->undo_manager()->updated_item(m_layer);
    }
}

EditorLayerSettingCommand::EditorLayerSettingCommand(model::Layer *layer, const model::LayerSetting &setting)
    : Command(CommandType::e_LayerSetting, "Layer Setting")
    , m_layer(layer)
    , m_setting(setting)
{
    m_old_setting = model::layer_setting(layer);
}

void EditorLayerSettingCommand::redo()
{
    model::apply_layer_setting(m_layer, m_setting);
    update_document();
}

void EditorLayerSettingCommand::undo()
{
    model::apply_layer_setting(m_layer, m_old_setting);
    update_document();
}

bool EditorLayerSettingCommand::merge_with(const Command *other)
{
    const EditorLayerSettingCommand *other_command = static_cast<const EditorLayerSettingCommand *>(
        other);
    if (m_layer == other_command->m_layer) {
        m_setting = other_command->m_setting;
        return true;
    }

    return false;
}

void EditorLayerSettingCommand::update_document()
{
    if (m_document) {
        m_document->undo_manager()->updated_item(m_layer, m_roles);
    }
}

AddRemoveShapeItemCommand::AddRemoveShapeItemCommand(
    model::ShapeLayer *layer, model::Group *group, model::ShapeItem *shape, int index, Type type)
    : LayerCommand(layer, CommandType::e_ShapeAddRemoveShape)
    , m_shape_layer(layer)
    , m_group(group)
    , m_shape(shape)
    , m_type(type)
    , m_index(index)
{}

void AddRemoveShapeItemCommand::redo()
{
    if (m_type == e_Add) {
        add_shape();
    } else {
        remove_shape();
    }
}

void AddRemoveShapeItemCommand::undo()
{
    if (m_type == e_Add) {
        remove_shape();
    } else {
        add_shape();
    }
}

void AddRemoveShapeItemCommand::add_shape()
{
    if (m_document) {
        if (m_type == e_Add) {
            m_index = m_document->undo_manager()->add_shape(m_shape_layer, m_group, m_shape);
        } else {
            m_document->undo_manager()->add_shape(m_shape_layer, m_group, m_shape, m_index);
        }
    }
}

void AddRemoveShapeItemCommand::remove_shape()
{
    if (m_document) {
        if (m_type == e_Remove) {
            m_index = m_document->undo_manager()->remove_shape(m_shape_layer, m_group, m_shape);
        } else {
            m_document->undo_manager()->remove_shape(m_shape_layer, m_group, m_shape);
        }
    }
}

SetLayerRangeCommand::SetLayerRangeCommand(model::Layer *layer, KeyFrameTime in, KeyFrameTime out)
    : LayerCommand(layer, CommandType::e_LayerSetRange)
    , m_in(in)
    , m_out(out)
{
    m_in_old = layer->in_point();
    m_out_old = layer->out_point();
    update_text();
}

void SetLayerRangeCommand::redo()
{
    m_layer->set_in_point(m_in);
    m_layer->set_out_point(m_out);
    if (m_document) {
        m_document->undo_manager()->updated_item(m_layer);
    }
}

void SetLayerRangeCommand::undo()
{
    m_layer->set_in_point(m_in_old);
    m_layer->set_out_point(m_out_old);
    if (m_document) {
        m_document->undo_manager()->updated_item(m_layer);
    }
}

bool SetLayerRangeCommand::merge_with(const Command *other)
{
    const SetLayerRangeCommand *other_command = static_cast<const SetLayerRangeCommand *>(other);
    if (m_layer == other_command->layer()) {
        m_in = other_command->in_point();
        m_out = other_command->out_point();
        update_text();
        return true;
    }
    return false;
}

void SetLayerRangeCommand::update_text() {}

MoveLayersCommand::MoveLayersCommand(int from, int to)
    : Command(CommandType::e_LayersMove, {})
    , m_from(from)
    , m_to(to)
{
}

void MoveLayersCommand::redo()
{
    if (m_document) {
        m_document->undo_manager()->move_layer(m_from, m_to);
    }
}

void MoveLayersCommand::undo()
{
    if (m_document) {
        int to = m_from;
        int from = m_to;
        if (m_to > m_from) {
            from -= 1;
        } else {
            to += 1;
        }

        m_document->undo_manager()->move_layer(from, to);
    }
}

bool MoveLayersCommand::merge_with(const Command *other)
{
    return false;
}

MoveShapesCommand::MoveShapesCommand(model::ShapeLayer *shape_layer,
                                     model::Object *object,
                                     int from,
                                     int to)
    : Command(CommandType::e_ShapesMove, {})
    , m_shape_layer(shape_layer)
    , m_object(object)
    , m_from(from)
    , m_to(to)
{
}

void MoveShapesCommand::redo()
{
    if (m_document) {
        m_document->undo_manager()->move_shape(m_shape_layer, m_object, m_from, m_to);
    }
}

void MoveShapesCommand::undo()
{
    if (m_document) {
        int to = m_from;
        int from = m_to;
        if (m_to > m_from) {
            from -= 1;
        } else {
            to += 1;
        }

        m_document->undo_manager()->move_shape(m_shape_layer, m_object, from, to);
    }
}

} // namespace alive
