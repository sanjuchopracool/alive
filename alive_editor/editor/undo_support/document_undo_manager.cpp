#include <core/model/composition.h>
#include <core/model/layers/shape_layer.h>
#include <core/model/layers/text_layer.h>
#include <core/model/property/easing.h>
#include <core/model/property/property_group.h>
#include <core/model/property/static_property_factory.h>
#include <core/model/shape_items/group.h>
#include <core/model/shape_items/shape_factory.h>
#include <core/model/shape_items/shape_item.h>
#include <editor/document/document.h>
#include <editor/interfaces/timeline.h>
#include <editor/undo_support/document_commands.h>
#include <editor/undo_support/document_undo_manager.h>
#include <editor/undo_support/functor_commands.h>
#include <editor/undo_support/keyframe_commands.h>
#include <editor/undo_support/layer_command.h>
#include <editor/undo_support/layer_commands.h>
#include <editor/undo_support/object_commands.h>
#include <editor/undo_support/property_commands.h>
#include <editor/undo_support/shape_item_commands.h>
#include <editor/undo_support/timeline_commands.h>

namespace alive {

DocumentUndoManager::DocumentUndoManager(Document *doc)
    : m_doc(doc)
    , m_comp(doc->composition())
    , m_tag_model(doc->tag_model())
{}

void DocumentUndoManager::redo()
{
    m_doc->m_document_undo_stack.redo();
}

void DocumentUndoManager::undo()
{
    m_doc->m_document_undo_stack.undo();
}

/*********************************************************************************
*                 Tagging Support
*********************************************************************************/
void DocumentUndoManager::tag(Taggable *to, Tag *tag)
{
    auto cmd = FunctorCommand::create_functor_command(
        [this, to, tag](alive::FunctorCommand *cmd) { m_tag_model->tag(to, tag); },
        [this, to, tag](alive::FunctorCommand *cmd) { m_tag_model->remove_tag(to, tag); });
    push_cmd(cmd);
}

void DocumentUndoManager::remove_tag(Taggable *from, Tag *tag)
{
    auto cmd = FunctorCommand::create_functor_command(
        [this, from, tag](alive::FunctorCommand *cmd) { m_tag_model->remove_tag(from, tag); },
        [this, from, tag](alive::FunctorCommand *cmd) { m_tag_model->tag(from, tag); });
    push_cmd(cmd);
}

void DocumentUndoManager::push_cmd(alive_ptr<Command> &cmd)
{
    m_doc->push_command(cmd);
}

void DocumentUndoManager::create_tag(const std::string &name)
{
    auto cmd = FunctorCommand::create_functor_command(
        [this, name](alive::FunctorCommand *cmd) {
            auto *tag = m_tag_model->create_tag(name);
            if (tag) {
                cmd->set_int(tag->id());
            } else {
                cmd->set_int(-1);
                // emit tag_already_exist(name);
            }
        },
        [this, name](alive::FunctorCommand *cmd) {
            auto id = cmd->get_int();
            if (id != -1) {
                m_tag_model->delete_tag(id);
                cmd->set_int(-1);
            }
        });
    push_cmd(cmd);
}

void DocumentUndoManager::delete_with_context(
    const std::vector<ITagModel::TagDeleteContext> &selection)
{
    auto cmd = FunctorCommand::create_functor_command(
        [this, selection](alive::FunctorCommand *cmd) {
            for (auto &data : selection) {
                Tag *tag = m_comp->tag_manager()->tag(data.tag_name);
                if (!tag)
                    continue;

                for (auto *entity : data.entities) {
                    m_tag_model->remove_tag(entity, tag);
                }

                if (data.delete_tag) {
                    m_tag_model->delete_tag(tag->id());
                }
            }
        },
        [this, selection](alive::FunctorCommand *cmd) {
            for (auto it = selection.rbegin(); it != selection.rend(); ++it) {
                auto &data = *it;

                Tag *tag = nullptr;
                if (data.delete_tag) {
                    tag = m_tag_model->create_tag(data.tag_name);
                } else {
                    tag = m_comp->tag_manager()->tag(data.tag_name);
                }

                if (!tag) {
                    continue;
                }

                for (auto *entity : data.entities) {
                    m_tag_model->tag(entity, tag);
                }
            }
        });
    push_cmd(cmd);
}

/*********************************************************************************
*                 Property Group Related
*********************************************************************************/

void DocumentUndoManager::delete_property(model::PropertyGroup *group, model::Property *prop)
{
    auto cmd = FunctorCommand::create_functor_command(
        [this, group, prop](alive::FunctorCommand *cmd) {
            int index = group->remove(prop);
            if (index != -1) {
                m_doc->emit_property_removed(group, prop, index);
            }
            cmd->set_int(index);
        },
        [this, group, prop](alive::FunctorCommand *cmd) {
            int index = cmd->get_int();

            if (index != -1) {
                group->add(prop, index);
                m_doc->emit_property_added(group, prop, index);
                cmd->set_int(-1);
            }
        },
        [this, prop](alive::FunctorCommand *cmd) {
            if (cmd->is_done()) {
                int index = cmd->get_int();
                if (index != -1) {
                    delete prop;
                }
            }
        });
    push_cmd(cmd);
}

void DocumentUndoManager::duplicate_property(model::PropertyGroup *group, model::Property *prop)
{
    auto cmd = FunctorCommand::create_functor_command(
        [this, group, prop](alive::FunctorCommand *cmd) {
            model::Property *new_prop = nullptr;
            int index = -1;

            auto *data = cmd->get_property_data();
            if (data && data->property) {
                new_prop = data->property;
                index = data->index;
            } else {
                if (!prop)
                    return;

                index = group->index_of(prop);
                if (index == -1)
                    return;

                index = index + 1;
                if (!new_prop) {
                    new_prop = prop->clone();
                }
            }

            if (new_prop && index != -1) {
                group->add(new_prop, index);
                m_doc->emit_property_added(group, new_prop, index);
                if (!data) {
                    FunctorCommand::PropertyData new_data;
                    new_data.property = new_prop;
                    new_data.index = index;
                    cmd->set_property_data(new_data);
                }
            }
        },
        [this, group](alive::FunctorCommand *cmd) {
            auto *data = cmd->get_property_data();
            if (data && data->property) {
                m_doc->emit_property_removed(group, data->property, data->index);
                group->remove(data->property);
            }
        },
        [this](alive::FunctorCommand *cmd) {
            if (cmd->is_done())
                return;

            auto *data = cmd->get_property_data();
            if (data && data->property) {
                delete data->property;
            }
        });
    push_cmd(cmd);
}

void DocumentUndoManager::move_up_property(model::PropertyGroup *group, model::Property *prop)
{
    auto cmd = FunctorCommand::create_functor_command(
        [this, group, prop](alive::FunctorCommand *cmd) {
            int index = group->index_of(prop);
            if (index <= 0) {
                cmd->set_int(-1);
                return;
            }

            group->remove(prop);
            m_doc->emit_property_removed(group, prop, index);
            group->add(prop, index - 1);
            m_doc->emit_property_added(group, prop, index - 1);
            cmd->set_int(index);
        },
        [this, group, prop](alive::FunctorCommand *cmd) {
            int index = cmd->get_int();
            if (index != -1) {
                group->remove(prop);
                m_doc->emit_property_removed(group, prop, index - 1);
                group->add(prop, index);
                m_doc->emit_property_added(group, prop, index);
                cmd->set_int(index);
            }
        });
    push_cmd(cmd);
}

void DocumentUndoManager::add_property(model::PropertyGroup *group,
                                       model::PropertyType type,
                                       const std::string &name,
                                       const std::string &id)
{
    auto cmd = FunctorCommand::create_functor_command(
        [this, group, type, name, id](alive::FunctorCommand *cmd) {
            model::Property *new_prop = nullptr;
            int index = -1;

            auto *data = cmd->get_property_data();
            if (data && data->property) {
                new_prop = data->property;
                index = data->index;
            } else {
                new_prop = model::StaticPropertyFactory::create_static_property(type);
                if (new_prop) {
                    new_prop->set_name(name);
                    new_prop->set_id(id);
                }
            }

            if (new_prop) {
                index = group->add(new_prop, index);
                m_doc->emit_property_added(group, new_prop, index);
                if (!data) {
                    FunctorCommand::PropertyData new_data;
                    new_data.property = new_prop;
                    new_data.index = index;
                    cmd->set_property_data(new_data);
                }
            }
        },
        [this, group](alive::FunctorCommand *cmd) {
            auto *data = cmd->get_property_data();
            if (data && data->property) {
                m_doc->emit_property_removed(group, data->property, data->index);
                group->remove(data->property);
            }
        },
        [this](alive::FunctorCommand *cmd) {
            if (cmd->is_done())
                return;

            auto *data = cmd->get_property_data();
            if (data && data->property) {
                delete data->property;
            }
        });
    push_cmd(cmd);
}

/*********************************************************************************
*                 Object Related
*********************************************************************************/

void DocumentUndoManager::set_object_visible(model::Object *object, bool visible)
{
    alive_ptr<Command> cmd
        = Corrade::Containers::pointer<SetObjectBooleanCommand>(SetObjectBooleanCommand::e_Visible,
                                                                visible,
                                                                object);
    push_cmd(cmd);
}

bool DocumentUndoManager::set_object_name(model::Object *obj,
                                          const std::string &name,
                                          std::vector<int> roles)
{
    if (obj && obj->can_rename()) {
        auto old_name = obj->name();
        if (name != old_name) {
            auto cmd = FunctorCommand::create_functor_command(
                [this, obj, name, roles](alive::FunctorCommand *cmd) {
                    obj->set_name(name);
                    this->updated_item(obj, roles);
                },
                [this, obj, old_name, roles](alive::FunctorCommand *cmd) {
                    obj->set_name(old_name);
                    this->updated_item(obj, roles);
                });
            push_cmd(cmd);
            return true;
        }
    }
    return false;
}

/*********************************************************************************
*                 Layer Related
*********************************************************************************/

void DocumentUndoManager::insert_new_layers(const std::vector<model::Layer *> &layers, int index)
{
    alive_ptr<Command> cmd
        = Corrade::Containers::pointer<AddRemoveLayerCommand>(layers,
                                                              AddRemoveLayerCommand::e_Add,
                                                              index);
    push_cmd(cmd);
}

void DocumentUndoManager::remove_layers(const std::vector<model::Layer *> &layers)
{
    alive_ptr<Command> cmd
        = Corrade::Containers::pointer<AddRemoveLayerCommand>(layers,
                                                              AddRemoveLayerCommand::e_Remove);
    push_cmd(cmd);
}

void DocumentUndoManager::set_layer_in_out_points(model::Layer *layer,
                                                  KeyFrameTime in_point,
                                                  KeyFrameTime out_point,
                                                  std::vector<int> roles)
{
    alive_ptr<Command> cmd = Corrade::Containers::pointer<SetLayerRangeCommand>(layer,
                                                                                in_point,
                                                                                out_point);
    cmd->set_roles(roles);
    push_cmd(cmd);
}

void DocumentUndoManager::set_layer_color_index(model::Layer *layer, int new_index)
{
    alive_ptr<Command> cmd
        = Corrade::Containers::pointer<SetLayerIntCommand>(SetLayerIntCommand::e_ColorIndex,
                                                           new_index,
                                                           layer);
    push_cmd(cmd);
}

void DocumentUndoManager::set_layer_lock(model::Layer *layer, bool lock)
{
    alive_ptr<Command> cmd
        = Corrade::Containers::pointer<SetLayerBooleanCommand>(SetLayerBooleanCommand::e_Lock,
                                                               lock,
                                                               layer);
    push_cmd(cmd);
}

void DocumentUndoManager::move_layers(int source_index, int destination_index)
{
    alive_ptr<Command> cmd = Corrade::Containers::pointer<MoveLayersCommand>(source_index,
                                                                             destination_index);
    push_cmd(cmd);
}

void DocumentUndoManager::set_layer_matte_type(model::MatteType change,
                                               model::MatteType old,
                                               model::Layer *layer,
                                               std::vector<int> roles)
{
    auto cmd = FunctorCommand::create_functor_command(
        [this, change, layer, roles](alive::FunctorCommand *cmd) {
            layer->set_matte_type(change);
            this->updated_item(layer, roles);
        },
        [this, old, layer, roles](alive::FunctorCommand *cmd) {
            layer->set_matte_type(old);
            this->updated_item(layer, roles);
        });
    push_cmd(cmd);
}

void DocumentUndoManager::set_layer_blend_mode(model::BlendMode change,
                                               model::BlendMode old,
                                               model::Layer *layer,
                                               std::vector<int> roles)
{
    auto cmd = FunctorCommand::create_functor_command(
        [this, change, layer, roles](alive::FunctorCommand *cmd) {
            layer->set_blend_mode(change);
            this->updated_item(layer, roles);
        },
        [this, old, layer, roles](alive::FunctorCommand *cmd) {
            layer->set_blend_mode(old);
            this->updated_item(layer, roles);
        });
    push_cmd(cmd);
}

void DocumentUndoManager::set_layer_matte_layer(int change,
                                                int old,
                                                model::Layer *layer,
                                                std::vector<int> roles)
{
    auto cmd = FunctorCommand::create_functor_command(
        [this, change, layer, roles](alive::FunctorCommand *cmd) {
            layer->set_matte_layer_index(change);
            this->updated_item(layer, roles);
        },
        [this, old, layer, roles](alive::FunctorCommand *cmd) {
            layer->set_matte_layer_index(old);
            this->updated_item(layer, roles);
        });
    push_cmd(cmd);
}

void DocumentUndoManager::set_layer_parent_layer(int change,
                                                 int old,
                                                 model::Layer *layer,
                                                 std::vector<int> roles)
{
    auto cmd = FunctorCommand::create_functor_command(
        [this, change, layer, roles](alive::FunctorCommand *cmd) {
            layer->set_parent_layer_index(change);
            this->updated_item(layer, roles);
        },
        [this, old, layer, roles](alive::FunctorCommand *cmd) {
            layer->set_parent_layer_index(old);
            this->updated_item(layer, roles);
        });
    push_cmd(cmd);
}

void DocumentUndoManager::update_text_layer_settings(model::TextLayer *text_layer,
                                                     const core::Text &text)
{
    alive_ptr<Command> cmd(create_dynamic_property_change_command<core::Text>(
        &text_layer->text(), m_doc->m_current_frame, text, [this, text_layer](const core::Text &val) {
            m_doc->emit_text_layer_settings_changed(text_layer, val);
        }));
    if (cmd) {
        push_cmd(cmd);
    }
}

void DocumentUndoManager::set_layer_setting(model::Layer* layer, const model::LayerSetting &setting,
                                            std::vector<int> roles)
{
    alive_ptr<Command> cmd = Corrade::Containers::pointer<EditorLayerSettingCommand>(layer, setting);
    if (cmd) {
        cmd->set_roles(roles);
        push_cmd(cmd);
    }
}

int DocumentUndoManager::add_layer(model::Layer *layer, int index)
{
    return m_doc->add_layer(layer, index);
}

int DocumentUndoManager::remove_layer(model::Layer *layer)
{
    return m_doc->remove_layer(layer);
}

/*********************************************************************************
*                 WORK AREA RELATED
*********************************************************************************/
void DocumentUndoManager::set_work_area_start(ITimeline *timeline, int frame, bool merge)
{
    if (!timeline)
        return;
    alive_ptr<Command> cmd
        = Corrade::Containers::pointer<TimelineRangeCommand>(timeline,
                                                             TimelineRangeCommand::e_WorkArea,
                                                             frame,
                                                             timeline->play_end(),
                                                             merge);
    push_cmd(cmd);
}

void DocumentUndoManager::set_work_area_end(ITimeline *timeline, int frame, bool merge)
{
    if (!timeline)
        return;

    alive_ptr<Command> cmd
        = Corrade::Containers::pointer<TimelineRangeCommand>(timeline,
                                                             TimelineRangeCommand::e_WorkArea,
                                                             timeline->play_start(),
                                                             frame,
                                                             merge);
    push_cmd(cmd);
}

void DocumentUndoManager::set_work_area(ITimeline *timeline, int start, int end, bool merge)
{
    if (!timeline)
        return;

    alive_ptr<Command> cmd
        = Corrade::Containers::pointer<TimelineRangeCommand>(timeline,
                                                             TimelineRangeCommand::e_WorkArea,
                                                             start,
                                                             end,
                                                             merge);
    push_cmd(cmd);
}

void DocumentUndoManager::set_visible_range_area(ITimeline *timeline, int start, int end, bool merge)
{
    if (!timeline)
        return;

    alive_ptr<Command> cmd
        = Corrade::Containers::pointer<TimelineRangeCommand>(timeline,
                                                             TimelineRangeCommand::e_Visible,
                                                             start,
                                                             end,
                                                             merge);
    push_cmd(cmd);
}

/*********************************************************************************
*                 Composition Setting
*********************************************************************************/
void DocumentUndoManager::update_composition_setting(const CompositionSetting &setting)
{
    if (setting != m_doc->composition_setting()) {
        alive_ptr<Command> cmd = Corrade::Containers::pointer<CompositionSettingCommand>(setting);
        push_cmd(cmd);
    }
}

void DocumentUndoManager::set_apply_script(const std::string &script)
{
    auto old_script = m_comp->apply_script();
    auto cmd = FunctorCommand::create_functor_command(
        [this, script](alive::FunctorCommand *cmd) { this->m_comp->set_apply_script(script); },
        [this, old_script](alive::FunctorCommand *cmd) { m_comp->set_apply_script(old_script); });
    push_cmd(cmd);
}

void DocumentUndoManager::updated_item(const TreeItem *item, std::vector<int> roles)
{
    m_doc->emit_updated_item(item, roles);
}

bool DocumentUndoManager::set_path_direction(model::PathShapeItem *item,
                                             bool value,
                                             std::vector<int> roles)
{
    PathDirection old_direction = item->path_direction();
    PathDirection direction = value ? PathDirection::e_Clockwise
                                    : PathDirection::e_CounterClockwise;
    if (old_direction != direction) {
        alive_ptr<Command> cmd
            = Corrade::Containers::pointer<SetPathShapeDirectionCommand>(item, direction);
        cmd->set_roles(roles);
        push_cmd(cmd);
        return true;
    }
    return false;
}

bool DocumentUndoManager::add_shape_item_to_shape_group(model::Object *object, int item_index)
{
    using namespace model;
    Group *group = object ? object->to_shape_group() : nullptr;
    if (group) {
        ShapeItem *shape_item = nullptr;
        ShapeLayer *shape_layer = nullptr;
        if (object) {
            shape_layer = static_cast<model::ShapeLayer *>(Document::layer_for_item(group));
            ShapeType shape_type = static_cast<ShapeType>(item_index);
            shape_item = ShapeFactory::create_shape_item(shape_type);
        }

        if (shape_item) {
            alive_ptr<Command> cmd
                = Corrade::Containers::pointer<AddRemoveShapeItemCommand>(shape_layer,
                                                                          group,
                                                                          shape_item,
                                                                          -1);
            push_cmd(cmd);
        }
        return shape_item;
    } else {
        ALIVE_CORE_DEBUG("Selected Object is not shape object!");
        return false;
    }
}

void DocumentUndoManager::remove_shape_item(model::ShapeItem *shape_item)
{
    if (shape_item) {
        auto shape_layer = static_cast<model::ShapeLayer *>(m_doc->layer_for_item(shape_item));
        auto group
            = const_cast<GroupTreeItem *>(shape_item->parent())->to_object()->to_shape_group();
        if (shape_layer && group) {
            alive_ptr<Command> cmd = Corrade::Containers::pointer<AddRemoveShapeItemCommand>(
                shape_layer, group, shape_item, -1, AddRemoveShapeItemCommand::e_Remove);
            push_cmd(cmd);
        }
    }
}

void DocumentUndoManager::move_shape_item(model::ShapeLayer *shape_layer,
                                          model::Object *object,
                                          int source_index,
                                          int destination_index)
{
    alive_ptr<Command> cmd = Corrade::Containers::pointer<MoveShapesCommand>(shape_layer,
                                                                             object,
                                                                             source_index,
                                                                             destination_index);
    push_cmd(cmd);
}

void DocumentUndoManager::set_expression_on_property(model::Property *property,
                                                     const std::string &expression,
                                                     KeyFrameTime time)
{
    alive_ptr<Command> cmd
        = Corrade::Containers::pointer<SetPropertyExpressionCommand>(property,
                                                                     expression,
                                                                     time < 0
                                                                         ? m_doc->m_current_frame
                                                                         : time);
    push_cmd(cmd);
}

void DocumentUndoManager::disable_property_expression(model::Property *property, bool disable)
{
    alive_ptr<Command> cmd
        = Corrade::Containers::pointer<PropertyDisableExpressionCommand>(property, disable);
    push_cmd(cmd);
}

void DocumentUndoManager::disable_keyframe_expression(model::KeyFrame *keyframe, bool disable)
{
    alive_ptr<Command> cmd
        = Corrade::Containers::pointer<KeyFrameDisableExpressionCommand>(keyframe, disable);
    push_cmd(cmd);
}

void DocumentUndoManager::move_keyframes(const std::set<model::KeyFrame *> &keyframes,
                                         KeyFrameTime t,
                                         std::vector<int> roles)
{
    alive_ptr<Command> cmd = Corrade::Containers::pointer<MoveKeyFramesCommand>(keyframes, t);
    cmd->set_roles(roles);
    push_cmd(cmd);
}

void DocumentUndoManager::property_remove_keyframe_at(model::Property *property,
                                                      KeyFrameTime t,
                                                      std::vector<int> roles)
{
    alive_ptr<Command> cmd = Corrade::Containers::pointer<RemoveKeyFrameCommand>(t, property);
    cmd->set_roles(roles);
    push_cmd(cmd);
}

void DocumentUndoManager::property_add_keyframe_at(model::Property *property,
                                                   KeyFrameTime t,
                                                   std::vector<int> roles)
{
    alive_ptr<Command> cmd = Corrade::Containers::pointer<AddKeyFrameCommand>(t, property);
    cmd->set_roles(roles);
    push_cmd(cmd);
}

void DocumentUndoManager::property_remove_all_keyframes(model::Property *property,
                                                        std::vector<int> roles)
{
    alive_ptr<Command> cmd = Corrade::Containers::pointer<RemoveAllKeyFramesCommand>(property);
    cmd->set_roles(roles);
    push_cmd(cmd);
}

void DocumentUndoManager::update_easing_between_keyrames(
    const model::EasingBetweenTwoFramesData &data)
{
    if (data.m_left_keyframe) {
        alive_ptr<Command> cmd = Corrade::Containers::pointer<KeyFrameEasingCommand>(data);
        push_cmd(cmd);
    }
}

bool DocumentUndoManager::set_linked(model::Property *property, bool value, std::vector<int> roles)
{
    if (property && property->is_linked() != value) {
        auto cmd = FunctorCommand::create_functor_command(
            [this, value, property, roles](alive::FunctorCommand *cmd) {
                property->set_linked(value);
                this->updated_item(property, roles);
            },
            [this, value, property, roles](alive::FunctorCommand *cmd) {
                property->set_linked(!value);
                this->updated_item(property, roles);
            });
        push_cmd(cmd);
        return true;
    }
    return false;
}

void DocumentUndoManager::update_property_change(model::Property *property,
                                                 KeyFrameTime time,
                                                 EditingField field,
                                                 int change,
                                                 std::vector<int> roles)
{
    PropertyCommand *cmd = nullptr;
    switch (property->type()) {
    case alive::model::PropertyType::e_NumPoints: {
        model::DynamicProperty<float> *d_prop = static_cast<model::DynamicProperty<float> *>(
            property);
        float old_val = d_prop->get_value();
        float val = static_cast<int>(old_val + change);
        if (d_prop->validate(val) && (val != old_val)) {
            cmd = new SetPropertyValueCommand<float>(d_prop, time, val);
        }
    } break;
    case model::PropertyType::e_Rotation: {
        model::DynamicProperty<float> *d_prop = static_cast<model::DynamicProperty<float> *>(
            property);
        if (field == EditingField::e_First)
            change *= 360.0;
        float old_val = d_prop->get_value();
        float val = old_val + change;
        d_prop->validate(val);
        if (val != old_val) {
            cmd = new SetPropertyValueCommand<float>(d_prop, time, val);
        }
    } break;
    case model::PropertyType::e_Opacity:
    case model::PropertyType::e_Roundness:
    case model::PropertyType::e_Float: {
        model::DynamicProperty<float> *d_prop = static_cast<model::DynamicProperty<float> *>(
            property);
        float old_val = d_prop->get_value();
        float val = old_val + change;
        d_prop->validate(val);
        if (val != old_val) {
            cmd = new SetPropertyValueCommand<float>(d_prop, time, val);
        }
    } break;
    case model::PropertyType::e_Anchor3D:
    case model::PropertyType::e_Position3D:
    case model::PropertyType::e_Scale3D: {
        model::DynamicProperty<Vec3D> *d_prop = static_cast<model::DynamicProperty<Vec3D> *>(
            property);
        Vec3D old_value = d_prop->get_value();
        Vec3D new_val = old_value;
        if (field != EditingField::e_None) {
            if (field == EditingField::e_First) {
                new_val.x() += change;
            } else if (field == EditingField::e_Second) {
                new_val.y() += change;
            }
            if (new_val != old_value) {
                d_prop->update_linked_value(old_value, new_val);
                cmd = new SetPropertyValueCommand<Vec3D>(d_prop, time, new_val);
            }
        }
    } break;
    case model::PropertyType::e_Position2D:
    case model::PropertyType::e_Anchor2D:
    case model::PropertyType::e_Scale2D:
    case model::PropertyType::e_Size2D: {
        model::DynamicProperty<Vec2D> *d_prop = static_cast<model::DynamicProperty<Vec2D> *>(
            property);
        Vec2D old_value = d_prop->get_value();
        Vec2D new_val = old_value;
        if (field != EditingField::e_None) {
            if (field == EditingField::e_First) {
                new_val.x() += change;
            } else if (field == EditingField::e_Second) {
                new_val.y() += change;
            }
            if (new_val != old_value) {
                d_prop->update_linked_value(old_value, new_val);
                cmd = new SetPropertyValueCommand<Vec2D>(d_prop, time, new_val);
            }
        }
    } break;
    default:
        break;
    }

    if (cmd) {
        cmd->set_roles(roles);
        alive_ptr<Command> alive_command(cmd);
        m_doc->push_command(alive_command);
    }
}

bool DocumentUndoManager::update_property_change(model::Property *property,
                                                 KeyFrameTime time,
                                                 EditingField field,
                                                 const std::string &new_value_str,
                                                 std::vector<int> roles)
{
    PropertyCommand *cmd = nullptr;
    float new_value = std::stof(new_value_str);
    switch (property->type()) {
    case alive::model::PropertyType::e_NumPoints: {
        model::DynamicProperty<float> *d_prop = static_cast<model::DynamicProperty<float> *>(
            property);
        float old_val = d_prop->get_value();
        float val = static_cast<int>(new_value);
        if (d_prop->validate(val) && (val != old_val)) {
            cmd = new SetPropertyValueCommand<float>(d_prop, time, val);
        }
    } break;
    case model::PropertyType::e_Rotation: {
        model::DynamicProperty<float> *d_prop = static_cast<model::DynamicProperty<float> *>(
            property);
        float old_value = d_prop->get_value();
        float val = old_value;
        if (field == EditingField::e_First) {
            new_value *= 360.0;
            val = (old_value - (static_cast<int>(old_value) / 360) * 360) + new_value;
        } else if (field == EditingField::e_Second) {
            val = ((static_cast<int>(old_value) / 360) * 360) + new_value;
        }
        d_prop->validate(val);
        if (val != old_value) {
            cmd = new SetPropertyValueCommand<float>(d_prop, time, val);
        }
    } break;
    case model::PropertyType::e_Opacity:
    case model::PropertyType::e_Roundness:
    case model::PropertyType::e_Float: {
        model::DynamicProperty<float> *d_prop = static_cast<model::DynamicProperty<float> *>(
            property);
        float val = new_value;
        float old_value = d_prop->get_value();
        d_prop->validate(val);
        if (val != old_value) {
            cmd = new SetPropertyValueCommand<float>(d_prop, time, val);
        }
    } break;
    case model::PropertyType::e_Anchor3D:
    case model::PropertyType::e_Position3D:
    case model::PropertyType::e_Scale3D: {
        model::DynamicProperty<Vec3D> *d_prop = static_cast<model::DynamicProperty<Vec3D> *>(
            property);
        Vec3D old_value = d_prop->get_value();
        Vec3D new_val = old_value;
        if (field != EditingField::e_None) {
            if (field == EditingField::e_First) {
                new_val.x() = new_value;
            } else if (field == EditingField::e_Second) {
                new_val.y() = new_value;
            }
            if (new_val != old_value) {
                d_prop->update_linked_value(old_value, new_val);
                cmd = new SetPropertyValueCommand<Vec3D>(d_prop, time, new_val);
            }
        }
    } break;
    case model::PropertyType::e_Position2D:
    case model::PropertyType::e_Anchor2D:
    case model::PropertyType::e_Scale2D:
    case model::PropertyType::e_Size2D: {
        model::DynamicProperty<Vec2D> *d_prop = static_cast<model::DynamicProperty<Vec2D> *>(
            property);
        Vec2D old_value = d_prop->get_value();
        Vec2D new_val = old_value;
        if (field != EditingField::e_None) {
            if (field == EditingField::e_First) {
                new_val.x() = new_value;
            } else if (field == EditingField::e_Second) {
                new_val.y() = new_value;
            }
            if (new_val != old_value) {
                d_prop->update_linked_value(old_value, new_val);
                cmd = new SetPropertyValueCommand<Vec2D>(d_prop, time, new_val);
            }
        }
    } break;
    default:
        break;
    }

    if (cmd) {
        cmd->set_roles(roles);
        alive_ptr<Command> alive_command(cmd);
        m_doc->push_command(alive_command);
    }
    return true;
}

void DocumentUndoManager::update_composition_setting_impl(const CompositionSetting &setting)
{
    m_doc->update_composition_setting_impl(setting);
}

int DocumentUndoManager::add_shape(model::ShapeLayer *shape_layer,
                                   model::Group *group,
                                   model::ShapeItem *shape,
                                   int shape_index)
{
    return m_doc->add_shape(shape_layer, group, shape, shape_index);
}

int DocumentUndoManager::remove_shape(model::ShapeLayer *shape_layer,
                                      model::Group *group,
                                      model::ShapeItem *shape)
{
    return m_doc->remove_shape(shape_layer, group, shape);
}

bool DocumentUndoManager::move_shape(model::ShapeLayer *shape_layer,
                                     model::Object *object,
                                     int source_index,
                                     int destination_index)
{
    return m_doc->move_shape(shape_layer, object, source_index, destination_index);
}

bool DocumentUndoManager::move_layer(int source_index, int destination_index)
{
    return m_doc->move_layer(source_index, destination_index);
}

template<typename T>
bool DocumentUndoManager::change_dynamic_property_value(model::Property *property,
                                                        KeyFrameTime time,
                                                        const T &value,
                                                        std::vector<int> roles)
{
    alive_ptr<Command> cmd(create_dynamic_property_change_command<T>(property, time, value));
    if (cmd) {
        cmd->set_roles(roles);
        push_cmd(cmd);
        return true;
    }
    return false;
}

template bool DocumentUndoManager::change_dynamic_property_value<Vec3D>(model::Property *property,
                                                                        KeyFrameTime time,
                                                                        const Vec3D &value,
                                                                        std::vector<int> roles);

} // namespace alive
