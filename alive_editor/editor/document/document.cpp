#include <Corrade/Containers/Pointer.h>
#include <core/model/composition.h>
#include <core/model/layers/shape_layer.h>
#include <core/model/property/object_descriptor.h>
#include <editor/document/document.h>
#include <editor/undo_support/command.h>
#include <editor/undo_support/document_undo_manager.h>
namespace alive {

Document::Document(model::Composition *comp)
    : m_comp(comp)
{}

Document::~Document() {}

void Document::init() {
    m_undo_manager = Corrade::Containers::pointer<DocumentUndoManager>(this);
}

void Document::set_saved_path(const std::string &path)
{
    m_saved_path = path;
    m_is_modified = false;
}

void Document::set_modified(bool status)
{
    if (m_is_modified != status) {
        m_is_modified = status;
        emit_document_modified();
    }
}

DocumentUndoManager *Document::undo_manager() {
    return m_undo_manager.get();
}

model::Composition *Document::composition()
{
    return m_comp.get();
}

const model::Composition *Document::composition() const {
    return m_comp.get();
}

int Document::add_layer(model::Layer *layer, int index) {
    emit_about_to_insert_layer(layer,index);
    m_comp->add_layer_at_index(layer, index);
    emit_inserted_layer(layer, index);
    return index;
}

int Document::remove_layer(model::Layer *layer) {
    int index = m_comp->index_of_layer(layer);
    if (index != -1) {
        emit_about_to_remove_layer(layer, index);
        model::Layer *removed = m_comp->remove_layer_at_index(index);
        m_comp->remove_matte_for_layers(removed->layer_index());
        assert(layer == removed);
        emit_removed_layer(layer, index);
    }
    return index;
}
CompositionSetting Document::composition_setting() const
{
    CompositionSetting setting;
    setting.name = m_comp->name();
    setting.fps = m_comp->framerate();
    setting.height = m_comp->height();
    setting.width = m_comp->width();
    setting.duration = (m_comp->out_point() - m_comp->in_point() + 0.5) / setting.fps;
    setting.document = this;
    setting.bg_color = m_comp->canvas_background_color();
    setting.show_bg_color = m_comp->show_background_color();
    return setting;
}

void Document::update_composition_setting_impl(const CompositionSetting &setting) {
    m_comp->set_name(setting.name);
    m_comp->set_width(setting.width);
    m_comp->set_height(setting.height);
    m_comp->set_in_point(0);
    m_comp->set_framerate(setting.fps);
    m_comp->set_out_point(setting.fps * setting.duration);
    m_comp->set_show_canvas_bg_color(setting.show_bg_color);
    m_comp->set_canvas_background_color(setting.bg_color);
    emit_composition_settings_updated();
}

int Document::add_shape(model::ShapeLayer *shape_layer,
                        model::Group *group,
                        model::ShapeItem *shape,
                        int shape_index)
{
    if (shape_index == -1) {
        shape_index = group->count();
    }

    emit_about_to_add_shape_item(shape_layer, group, shape, shape_index);
    group->add_shape_item_at_index(shape, shape_index);
    emit_shape_item_added(shape_layer, group, shape, shape_index);
    return shape_index;
}

int Document::remove_shape(model::ShapeLayer *shape_layer,
                           model::Group *group,
                           model::ShapeItem *shape)
{
    int shape_index = group->index_of_item(shape);
    if (shape_index != -1) {
        emit_about_to_remove_shape(shape_layer, group, shape, shape_index);
        group->remove_shape_item_at_index(shape_index);
        emit_shape_removed(shape_index);
    }
    return shape_index;
}

bool Document::move_shape(model::ShapeLayer *shape_layer,
                          model::Object *object,
                          int source_index,
                          int destination_index)
{
    bool result = false;
    if (emit_about_to_move_shape(shape_layer, object, source_index, destination_index)) {
        if (destination_index > source_index) {
            destination_index -= 1;
        }
        result = shape_layer->move_shapes(object, source_index, destination_index);
        emit_shape_moved(shape_layer, object, source_index, destination_index);
    } else {
        ALIVE_CORE_DEBUG("Move Cancelled");
    }
    return result;
}

model::Layer *Document::layer_for_item(const TreeItem *item)
{
    if (!item)
        return nullptr;

    const model::Object *parent = nullptr;
    if (item->is_group()) {
        parent = static_cast<const model::Object *>(item);
    } else {
        parent = static_cast<const model::Object *>(item->parent());
    }
    while (parent && parent->object_type() != model::ObjectDescriptor::e_Layer) {
        parent = static_cast<const model::Object *>(parent->parent());
    }
    return const_cast<model::Layer *>(static_cast<const model::Layer *>(parent));
}

bool Document::move_layer(int source_index, int destination_index)
{
    bool result = false;
    if (emit_about_to_move_layer(source_index, destination_index)) {
        if (destination_index > source_index) {
            destination_index -= 1;
        }
        result = m_comp->move_layer(source_index, destination_index);
        emit_moved_layers(source_index, destination_index);
    }
    return result;
}

void Document::push_command(alive_ptr<Command> &cmd)
{
    if (cmd) {
        cmd->set_document(this);
        m_document_undo_stack.push_command(std::move(cmd));
    }
}
} // namespace alive
