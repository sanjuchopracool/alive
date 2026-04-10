#include "composition_document.h"
#include "factories/editor_layer_factory.h"
#include <core/composition/composition_node.h>
#include <core/model/composition.h>
#include <core/model/layers/layer.h>
#include <core/model/layers/precomposition_layer.h>
#include <core/model/layers/shape_layer.h>
#include <core/model/layers/text_layer.h>
#include <core/model/property/object.h>
#include <core/model/property/property.h>
#include <core/model/property/property_group.h>
#include <core/model/property/static_property_factory.h>
#include <core/model/shape_items/shape_factory.h>
#include <gui/color_utility.h>
#include <core/model/property/easing.h>
#include <gui/model/layers/gui_settings.h>
#include <gui/model/property/property_editor_callbacks.h>
#include <gui/theme/layer_color_model.h>
#include <gui/timeline_model_helpers.h>
#include <gui/utility/gui_utility.h>
#include <editor/undo_support/document_undo_manager.h>
#include <QIcon>
#include <QUndoStack>

namespace alive::model {

namespace {
const QString &k_none_string("None");
}

QString display_role_value(const Property *property, const Object *object, const Layer *layer)
{
    if (layer) {
        return layer->name().c_str();
    }

    if (object) {
        auto obj_name = object->name();
        if (!obj_name.empty()) {
            return obj_name.c_str();
        }
    }

    return property ? property->property_name().c_str() : QString();
}

QString label_role_value(const Property *property, const Object *object, const Layer *layer)
{
    if (layer) {
        // return alive::gui::editor_color_at_index(
        //            layer->editor_color_index())
        //     .name;
        return {};
    }
    if (object) {
        auto obj_name = object->name();
        if (!obj_name.empty()) {
            return obj_name.c_str();
        }
    }
    return property ? property->property_name().c_str() : QString();
}

int layer_color_index(const Property *property, const Object *object, const Layer *layer)
{
    if (layer) {
        layer->editor_color_index();
    }
    return 0;
}

QColor layer_edit_color(const Property *property, const Object *object, const Layer *layer)
{
    if (layer) {
        return alive::gui::editor_color_at_index(layer->editor_color_index()).color;
    }
    return {};
}

bool visible_role_value(const Property *property, const Object *object, const Layer *layer)
{
    if (object) {
        return object->is_object_visible();
    }
    return false;
}

bool CompositionDocument::locked_role_value(const Property *property,
                                            const Object *object,
                                            const Layer *layer) const
{
    if (!layer) {
        const TreeItem *item = nullptr;
        if (property) {
            item = property;
        } else {
            item = object;
        }
        layer = layer_for_item(item);
    }
    return layer->is_locked();
}
bool CompositionDocument::layer_blink_role_value(const Property *property,
                                                 const Object *object,
                                                 const Layer *layer) const
{
    if (layer) {
        return layer->is_blinking();
    }

    return false;
}

QVariant CompositionDocument::drag_status_value(const Property *property,
                                                const Object *object,
                                                const Layer *layer) const
{
    DragSupportStatusValue status;
    if (object) {
        status.m_support_drag = object->is_layer() || object->is_shape_item();
        status.m_support_drop = object->is_layer() || object->is_shape_item()
                                || object->is_content_group();
    }

    QVariant data;
    data.setValue(status);
    return data;
}

QVariant CompositionDocument::item_data(const TreeItem *item,
                                        CompositionItemModel::Section /*section*/,
                                        int role) const
{
    const Property *property = item->to_property();
    const Object *object = item->to_object();
    const Layer *layer = item->to_layer();

    switch (role) {
    case Qt::DisplayRole:
        return display_role_value(property, object, layer);
        break;
    case CompositionDocument::DragSupportStatus:
        return drag_status_value(property, object, layer);
        break;
    case CompositionDocument::LabelText:
        return label_role_value(property, object, layer);
        break;
    case CompositionDocument::IsLayer:
        return {layer != nullptr};
        break;
    case CompositionDocument::IsObject:
        return {object != nullptr};
        break;
    case CompositionDocument::LayerColorIndex:
        return layer_color_index(property, object, layer);
        break;
    case CompositionDocument::LayerColor:
        return layer_edit_color(property, object, layer);
        break;
    case CompositionDocument::LayerLocked:
        return locked_role_value(property, object, layer);
        break;
    case CompositionDocument::ObjectVisible:
        return visible_role_value(property, object, layer);
        break;
    case CompositionDocument::LayerBlink:
        return layer_blink_role_value(property, object, layer);
        break;
    case CompositionDocument::EditValue:
        return gui::Utility::instance()->edit_value(property, object, layer);
        break;
    case CompositionDocument::IsLinked:
        return property ? property->is_linked() : false;
        break;
    case CompositionDocument::IsLinkable:
        return property ? property->linkable() : false;
        break;
    case CompositionDocument::CanAnimate:
        return property ? property->can_animate() : false;
        break;
    case CompositionDocument::CanDelete:
        return object ? object->can_delete() : false;
        break;
    case CompositionDocument::IsAnimating:
        return property ? property->is_animating() : false;
        break;
    case CompositionDocument::ExprEnabled:
        return property ? property->expression_enabled() : false;
        break;
    case CompositionDocument::KeyFrameCount: {
        return property ? property->keyframes_count() : false;
    } break;
    case CompositionDocument::ItemRole: {
        QVariant data;
        data.setValue(const_cast<TreeItem *>(item));
        return data;
    } break;
    case CompositionDocument::KeyFrameStatus: {
        return get_property_keyframe_status(property, m_current_frame);
    } break;
    case CompositionDocument::LayerInOutRange: {
        if (layer) {
            LayerInOutPoints range;
            range.in_point = layer->in_point();
            range.out_point = layer->out_point();
            QVariant data;
            data.setValue(range);
            return data;
        }
    } break;
    case CompositionDocument::ObjectSupportVisibility: {
        if (object) {
            return object->support_visibility();
        }
        return false;
    } break;
    default:
        break;
    }

    return {};
}

bool CompositionDocument::blink_if_locked(TreeItem *property)
{
    Layer *layer = nullptr;
    layer = layer_for_item(property);
    auto editor_layer = layer;

    if (editor_layer->is_locked()) {
        editor_layer->set_blink(true);
        updated_property_role(layer, LayerBlink);
        return true;
    }

    return false;
}

std::pair<Layer *, LayerSetting> CompositionDocument::create_and_push_new_layer(LayerType type)
{
    auto result = create_new_layer(type);
    if (result.first) {
        add_new_layers_command({result.first});
    }
    return result;
}

std::pair<Layer *, LayerSetting> CompositionDocument::create_new_layer(LayerType type)
{
    auto result = std::make_pair<Layer *, LayerSetting>(nullptr, LayerSetting());
    EditorLayerFactory::CreateInfo create_data;
    create_data.first = type;
    create_data.second = m_new_layer_index++;
    auto layer_data = EditorLayerFactory::create_layer(create_data, *m_comp);
    if (layer_data.first) {
        auto layer = layer_data.first.release();
        result.first = layer;
        result.second = layer_data.second;
    }
    return result;
}

void CompositionDocument::reset_uuid()
{
    m_comp->set_uuid(QUuid::createUuid().toString().toStdString());
}

void CompositionDocument::update_layer_index_info()
{
    m_layers_index_info.clear();
    static const QString name_key = "name";
    static const QString index_key = "index";
    QVariantMap layer_info;
    layer_info[name_key] = k_none_string;
    layer_info[index_key] = -1;
    m_layers_index_info.append(layer_info);
    m_layers_index_to_name.clear();
    m_layers_index_to_name[-1] = k_none_string;
    for (const auto &layer : m_comp->layers()) {
        QVariantMap layer_info;
        const QString &name = QString::fromStdString(layer->name());
        layer_info[name_key] = name;
        layer_info[index_key] = layer->layer_index();
        m_layers_index_info.append(layer_info);
        m_layers_index_to_name[layer->layer_index()] = name;
    }

    emit layers_index_info_changed();
}

bool CompositionDocument::set_item_data(TreeItem *item, int role, const QVariant &data)
{
    Property *property = item->to_property();
    Object *object = item->to_object();
    Layer *layer = item->to_layer();

    if (role != LayerBlink && role != LayerLocked && role != ObjectVisible) {
        Layer *property_layer = layer;
        if (!layer) {
            property_layer = layer_for_item(item);
        }
        /*
        if (role == LayerLocked) {
            bool new_state = data.toBool();
            if (layer && new_state != editor_layer->is_locked()) {
                set_layer_lock(layer, new_state);
                return true;
            }
        } else if (role == LayerVisible) {
            bool new_state = data.toBool();
            if (layer && new_state != layer->is_visible()) {
                layer->set_visible(new_state);
                return true;
            }
        } else */

        if (property_layer->is_locked()) {
            property_layer->set_blink(true);
            updated_property_role(property_layer, LayerBlink);
            return true;
        }
    }
    switch (role) {
    case SetName:
        return undo_manager()->set_object_name(object, data.toString().toStdString(), std::vector<int>{Qt::DisplayRole});
        break;

        //    case e_IsLayer:
        //        return is_layer_role_value(property, object, layer);
        //        break;
    case LayerColorIndex: {
        if (data.isValid() && layer) {
            int new_index = data.toInt();
            if (new_index != layer->editor_color_index()) {
                undo_manager()->set_layer_color_index(layer, new_index);
                return true;
            }
        }
    } break;
    case LayerLocked: {
        if (layer) {
            bool new_state = data.toBool();
            if (new_state != layer->is_locked()) {
                undo_manager()->set_layer_lock(layer, new_state);
                return true;
            }
        }
    } break;
    case ObjectVisible: {
        if (object) {
            bool new_state = data.toBool();
            if (new_state != object->is_object_visible()) {
                m_undo_manager->set_object_visible(object, new_state);
                return true;
            }
        }
    } break;
    case LayerBlink: {
        model::Layer *editor_layer = nullptr;
        if (layer) {
            editor_layer = layer;
        } else {
            editor_layer = layer_for_item(property);
        }
        if (editor_layer) {
            bool new_state = data.toBool();
            if (new_state != editor_layer->is_blinking()) {
                editor_layer->set_blink(new_state);
                return true;
            }
        }
    } break;
    case ChangeValueT1: {
        if (!object) {
            gui::update_property_change(property,
                                        m_current_frame,
                                        alive::EditingField::e_First,
                                        this,
                                        data.toReal());
            return true;
        }
    } break;
    case ChangeValueT2: {
        if (!object) {
            gui::update_property_change(property,
                                        m_current_frame,
                                        alive::EditingField::e_Second,
                                        this,
                                        data.toReal());
            return true;
        }
    } break;
    case SetValueTextT1: {
        if (!object) {
            gui::update_property_change(property,
                                        m_current_frame,
                                        alive::EditingField::e_First,
                                        this,
                                        data.toString());
            return true;
        }
    } break;
    case SetValueTextT2: {
        if (!object) {
            gui::update_property_change(property,
                                        m_current_frame,
                                        alive::EditingField::e_Second,
                                        this,
                                        data.toString());
            return true;
        }
    } break;
    case AddShapeItem: {
        if (object) {
            return undo_manager()->add_shape_item_to_shape_group(object, data.toInt());
        }
    } break;
    case SetInPoint: {
        if (layer) {
            KeyFrameTime in = data.toInt();
            if (in != layer->in_point()) {
                set_layer_in_out_points(layer, in, layer->out_point());
                return true;
            }
        }
    } break;
    case SetOutPoint: {
        if (layer) {
            KeyFrameTime out = data.toInt();
            if (out != layer->out_point()) {
                set_layer_in_out_points(layer, layer->in_point(), out);
                return true;
            }
        }
    } break;
    case MoveInOutPoint: {
        if (layer) {
            KeyFrameTime change = data.toInt();
            if (change) {
                set_layer_in_out_points(layer,
                                        layer->in_point() + change,
                                        layer->out_point() + change);
                return true;
            }
        }
    } break;
    case SetMatteType: {
        if (layer) {
            MatteType change = static_cast<MatteType>(data.toInt());
            MatteType old = layer->matte_type();
            if (change != old) {
                undo_manager()->set_layer_matte_type(change, old, layer, std::vector<int>{EditValue});
                return true;
            }
        }
    } break;
    case SetBlendMode: {
        if (layer) {
            BlendMode change = static_cast<BlendMode>(data.toInt());
            BlendMode old = layer->blend_mode();
            if (change != old) {
                undo_manager()->set_layer_blend_mode(change, old, layer, std::vector<int>{EditValue});
                return true;
            }
        }
    } break;
    case SetMatteLayer: {
        if (layer) {
            int change = data.toInt();
            int old = layer->matte_layer_index();
            if (change != old) {
                undo_manager()->set_layer_matte_layer(change, old, layer, std::vector<int>{EditValue});
                return true;
            }
        }
    } break;
    case SetParentLayer: {
        if (layer) {
            int change = data.toInt();
            int old = layer->parent_layer_index();
            if (change != old) {
                undo_manager()->set_layer_parent_layer(change, old, layer, std::vector<int>{EditValue});
                return true;
            }
        }
    } break;
    case EditValue: {
        return gui::Utility::instance()
            ->set_edit_value(this, property, object, layer, m_current_frame, data);
    } break;
    case AddKeyFrame: {
        if (property) {
            property_add_keyframe_at(property, m_current_frame);
            return true;
        }
        return false;
    } break;
    case RemoveKeyFrame: {
        if (property) {
            property_remove_keyframe_at(property, m_current_frame);
            return true;
        }
        return false;
    } break;
    case RemoveAllKeyFrame: {
        if (property) {
            property_remove_all_keyframes(property);
            return true;
        }
        return false;
    } break;
    case SetLinked: {
        if (property) {
            return undo_manager()->set_linked(property, data.toBool(), std::vector<int>{EditValue});
        }
        return false;
    } break;
    default:
        break;
    }

    return false;
}

CompositionDocument::CompositionDocument(Composition *comp, QObject *parent)
    : CompositionItemModel(comp, parent)
    , m_tag_model(new TagModel(comp->tag_manager(), this))
{
    if (m_comp->get_uuid().empty()) {
        reset_uuid();
    }

    observe(m_comp.get());
    m_new_layer_index = comp->layers_max_index();
    update_layer_index_info();

    Document::init();
}

CompositionDocument::~CompositionDocument() {}

void CompositionDocument::updated_index(const QModelIndex &index)
{
    emit dataChanged(index, index);
}

void CompositionDocument::add_new_layers_command(const std::vector<model::Layer *> &layers,
                                                 int index)
{
    using namespace commands;
    m_undo_manager->insert_new_layers(layers, index);
    update_layer_index_info();
}

void CompositionDocument::add_remove_layer_command(Layer *layer)
{
    m_undo_manager->remove_layers(std::vector{layer});
}

void CompositionDocument::updated_item(const TreeItem *property, QList<int> roles)
{
    const TreeItem *obj = property->parent();
    int left_column = 0;
    int right_column = CompositionItemModel::e_SectionCount - 1;
    const int prop_index = obj->index_of_item(property);
    QModelIndex left_index = createIndex(prop_index, 0, property);
    QModelIndex right_index = createIndex(prop_index, right_column, property);
    emit dataChanged(left_index, right_index, roles);
}

QModelIndex CompositionDocument::updated_property_role(const TreeItem *property, int role)
{
    QList<int> roles = QList<int>();
    roles << role;
    const TreeItem *obj = property->parent();
    QModelIndex left_index = createIndex(obj->index_of_item(property), 0, property);
    //    QModelIndex right_index = createIndex(obj->index_of_property(property), right_column, property);
    emit dataChanged(left_index, left_index, roles);
    return left_index;
}

void CompositionDocument::set_layer_in_out_points(Layer *layer,
                                                  KeyFrameTime in_point,
                                                  KeyFrameTime out_point)
{
    undo_manager()->set_layer_in_out_points(layer,
                                            in_point,
                                            out_point,
                                            std::vector<int>{LayerInOutRange});
}

TagModel *CompositionDocument::real_tag_model() const {
    return m_tag_model;
}

alive::CompositionDocumentProxyModel *CompositionDocument::proxy_model()
{
    return nullptr;
}

QVariant CompositionDocument::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (static_cast<Section>(section)) {
        case Section::e_Features:
            return tr("A/V Features");
        case Section::e_Layer:
            return tr("Source Name");
        case Section::e_Edit:
            return tr("Edit");
        case Section::e_TimeLine:
            return tr("TimeLine");
        default:
            break;
        }
    }

    if (section == e_Tree) {
        if (role == Qt::DecorationRole) {
            static QIcon label = QIcon::fromTheme("label");
            return label;
        }
    }

    return {};
}

void CompositionDocument::on_update(ISubject *s)
{
    if (!is_modified()) {
        set_modified(true);
        emit modified_changed(this);
    }

    if (m_comp->property_group() == s && m_comp->apply_script_on_change()
        && m_comp->apply_script().size()) {
        m_is_applying_script = true;
        emit apply_composition_script();
        m_is_applying_script = false;
        emit applied_script();
    }
}

QHash<int, QByteArray> CompositionDocument::roleNames() const
{
    auto roles = QAbstractItemModel::roleNames();
    roles[IsLayer] = "IsLayer";
    roles[IsObject] = "IsObject";
    roles[LayerColorIndex] = "LayerColorIndex";
    roles[LabelText] = "LabelText";
    roles[LayerColor] = "LayerColor";
    roles[LayerLocked] = "LayerLocked";
    roles[LayerBlink] = "LayerBlink";
    roles[ChangeValueT1] = "ChangeValueT1";
    roles[ChangeValueT2] = "ChangeValueT2";
    roles[SetValueTextT1] = "SetValueTextT1";
    roles[SetValueTextT2] = "SetValueTextT2";
    roles[IsAnimating] = "IsAnimating";
    roles[CanAnimate] = "CanAnimate";
    roles[ItemRole] = "ItemRole";
    roles[KeyFrameCount] = "KeyFrameCount";
    roles[KeyFrameStatus] = "KeyFrameStatus";
    roles[EditValue] = "EditValue";
    roles[AddShapeItem] = "AddShapeItem";
    roles[DragSupportStatus] = "DragSupportStatus";
    roles[LayerInOutRange] = "LayerInOutRange";
    roles[SetInPoint] = "SetInPoint";
    roles[SetOutPoint] = "SetOutPoint";
    roles[MoveInOutPoint] = "MoveInOutPoint";
    roles[ObjectSupportVisibility] = "ObjectSupportVisibility";
    roles[ObjectVisible] = "ObjectVisible";
    roles[SetMatteType] = "SetMatteType";
    roles[SetMatteLayer] = "SetMatteLayer";
    roles[SetParentLayer] = "SetParentLayer";
    roles[SetBlendMode] = "SetBlendMode";
    return roles;
}

QVariant CompositionDocument::data(const QModelIndex &index, int role) const
{
    QVariant result;
    if (!index.isValid())
        return result;

    Section section = static_cast<Section>(index.column());
    const TreeItem *item = static_cast<TreeItem *>(index.internalPointer());
    if (item) {
        result = item_data(item, section, role);
    }
    return result;
}

bool CompositionDocument::setData(const QModelIndex &index, const QVariant &data, int role)
{
    bool result = false;
    if (!index.isValid() || !data.isValid())
        return result;

    TreeItem *item = static_cast<TreeItem *>(index.internalPointer());
    result = set_item_data(item, role, data);
    if (result) {
        //        Object *object = nullptr;
        //        if (property->type() == PropertyDescriptor::e_Object) {
        //            object = static_cast<Object *>(property);
        //        }

        QList<int> roles = QList<int>();
        /*        if (object->is_layer() && role == CustomRoles::LayerLocked) {
            int layer_index = m_comp->index_of_property(property);
            QModelIndex top_left = createIndex(layer_index, 0, property);
            Object *last_child_object = object;
            int last_child_index = last_child_object->property_count() - 1;
            Property *last_prop = object->property_at(last_child_index);
            while (last_prop->type() == PropertyDescriptor::e_Object) {
                last_child_object = static_cast<Object *>(last_prop);
                last_child_index = last_child_object->property_count() - 1;
                last_prop = last_child_object->property_at(last_child_index);
            }
            QModelIndex bottom_right = createIndex(last_child_index, 0, last_prop);
            roles << role;
            emit dataChanged(top_left, bottom_right, roles);
        } else*/
        {
            if (role >= ChangeValueT1 && role <= SetValueTextT2) {
            } else {
                roles << role;
            }
            emit dataChanged(index, index, roles);
        }
    }
    return result;
}

QString CompositionDocument::name() const
{
    QString name = QString::fromStdString(m_comp->name());
    return name.isEmpty() ? "Untitled" : name;
}

void CompositionDocument::set_name(const QString &name)
{
    m_comp->set_name(name.toStdString());
    emit name_changed(name);
}

QUuid CompositionDocument::id() const
{
    return QUuid::fromString(m_comp->get_uuid());
}

void CompositionDocument::set_id(const QUuid &uuid)
{
    m_comp->set_uuid(uuid.toString().toStdString());
    emit id_changed(uuid);
}

void CompositionDocument::container_nodes_updated()
{
    emit nodes_updated();
}

void CompositionDocument::move_items(TreeItem *from, TreeItem *to, bool above)
{
    // AD allow drag and drop between same layer
    // for shape layer allow to drop under content item or group
    if (from && to && (from != to)) {
        Object *to_object = nullptr;
        if (from->is_group()) {
            to_object = static_cast<Object *>(to);
            if (to_object->object_type() == ObjectDescriptor::e_Layer) {
            }
        }

        Object *from_object = nullptr;
        if (from->is_group()) {
            from_object = static_cast<Object *>(from);
            if (from_object->object_type() == ObjectDescriptor::e_Layer) {
            }
        }

        if (!from_object || !to_object)
            return;

        auto get_move_index_from_same_parent = [&above](const TreeItem *from, const TreeItem *to) {
            const TreeItem *object = from->parent();
            int from_index = object->index_of_item(from);
            int to_index = object->index_of_item(to);
            qDebug() << "Moving from index " << from_index << " to " << (to_index)
                     << (above ? "Above" : "Below");
            bool from_larger = from_index > to_index;
            if (from_larger) {
                if (above) {
                } else {
                    to_index += 1;
                }
            } else {
                if (above) {
                    //                    to_index -= 1;
                } else {
                    to_index += 1;
                }
            }

            if (to_index < 0) {
                to_index = 0;
            }

            qDebug() << "Final move from index " << from_index << " to " << to_index;
            //            if (from_index > to_index) {
            //                std::swap(from_index, to_index);
            //            }
            return std::make_pair(from_index, to_index);
        };
        if (from_object->is_layer() && to_object->is_layer()) {
            auto [source_index, destination_index] = get_move_index_from_same_parent(from, to);
            if (source_index != destination_index) {
                undo_manager()->move_layers(source_index, destination_index);
            }
            return;
        }
        // check if layer for both objects is same
        Layer *from_layer = layer_for_item(from_object);
        if (!from_object->is_layer() && !to_object->is_layer()
            && (from_layer == layer_for_item(to_object))) {
            // both are objects
            // drop on group always move to last
            Object *from_parent_object = const_cast<Object *>(
                static_cast<const Object *>(from_object->parent()));
            if (from_parent_object == to_object->parent()) {
                // move with in same group
                qDebug() << "Move between same group";
                auto [source_index, destination_index] = get_move_index_from_same_parent(from, to);
                if (source_index != destination_index) {
                    if (from_layer->layer_type() == LayerType::e_Shape) {
                        ShapeLayer *shape_layer = static_cast<ShapeLayer *>(from_layer);
                        using namespace commands;
                        undo_manager()->move_shape_item(shape_layer,
                                                        from_parent_object,
                                                        source_index,
                                                        destination_index);
                    }
                }
            } else {
                // move between different groups
                qDebug() << "Move between differnt group not supported yet";
            }
        }
    }
}

const QString &CompositionDocument::layer_name(int layer_index) const
{
    auto it = m_layers_index_to_name.find(layer_index);
    if (it != m_layers_index_to_name.end()) {
        return it->second;
    }
    //return none
    return k_none_string;
}

const std::map<int, QString> &CompositionDocument::layer_index_to_name_map() const
{
    return m_layers_index_to_name;
}

void CompositionDocument::set_show_matte_details(bool show)
{
    if (m_comp->show_matte_details() != show) {
        m_comp->set_show_matte_details(show);
        emit show_matte_details_changed(show);
    }
}

bool CompositionDocument::show_matte_details() const
{
    return m_comp->show_matte_details();
}

void CompositionDocument::delete_item(TreeItem *item)
{
    if (!item)
        return;

    auto layer = item->to_layer();
    if (layer) {
        add_remove_layer_command(layer);
    } else {
        auto obj = item->to_object();
        if (!obj || !obj->can_delete())
            return;

        if (obj->is_shape_item()) {
            auto shape = static_cast<ShapeItem *>(obj);
            if (shape) {
                undo_manager()->remove_shape_item(shape);
            }
        }
    }
}

void CompositionDocument::clone_items(std::set<TreeItem *> items)
{
    std::vector<Layer *> layers;
    for (auto *item : items) {
        auto layer = item->to_layer();
        if (layer) {
            layers.emplace_back(layer);
        }
    }

    auto compare_layers = [this](const Layer *l1, const Layer *l2) {
        return m_comp->index_of_layer(l1) < m_comp->index_of_layer(l2);
    };

    std::sort(layers.begin(), layers.end(), compare_layers);
    std::vector<Layer *> cloned_layers;
    for (auto layer : layers) {
        auto new_layer = layer->clone_layer(nullptr, m_new_layer_index++);
        if (new_layer) {
            new_layer->set_name(layer_type_name(new_layer->layer_type()) + "_"
                                + std::to_string(new_layer->layer_index()));
            new_layer->set_editor_color_index(
                gui::LayerColorModel::instance()->get_random_color_index());
            cloned_layers.emplace_back(new_layer);
        }
    }

    // add all layers together at front
    if (cloned_layers.size()) {
        add_new_layers_command(cloned_layers, 0);
    }
}

void CompositionDocument::move_keyframes(const std::set<KeyFrame *> &keyframes, KeyFrameTime t)
{
    undo_manager()->move_keyframes(keyframes, t, std::vector<int>{IsAnimating, KeyFrameCount});
}

QVariant CompositionDocument::get_property_keyframe_status(const Property *property,
                                                           KeyFrameTime time) const
{
    QVariant result;
    if (property) {
        KeyFrameStatusValue status = property->keyframe_status(time);
        result.setValue(status);
    }
    return result;
}

void CompositionDocument::property_remove_keyframe_at(Property *property, KeyFrameTime time)
{
    if (!property)
        return;

    if (blink_if_locked(property))
        return;

    undo_manager()->property_remove_keyframe_at(property ,time, std::vector<int>{IsAnimating, KeyFrameCount});
}

void CompositionDocument::property_add_keyframe_at(Property *property, KeyFrameTime time)
{
    if (!property)
        return;

    if (blink_if_locked(property))
        return;

    undo_manager()->property_add_keyframe_at(property ,time, std::vector<int>{IsAnimating, KeyFrameCount});
}

void CompositionDocument::property_remove_all_keyframes(Property *property)
{
    if (!property)
        return;

    if (blink_if_locked(property))
        return;

    undo_manager()->property_remove_all_keyframes(property, std::vector<int>{IsAnimating, KeyFrameCount});
}

void CompositionDocument::invoke_context_menu_for_property(TreeItem *item)
{
    if (item) {
        Object *object = item->to_object();
        Layer *layer = item->to_layer();

        if (layer) {
            emit show_menu_for_layer(layer);
        } else if (object) {
            ObjectDescriptor::ObjectType obj_type = object->object_type();
            if (obj_type > ObjectDescriptor::e_ShapeItemStart
                && obj_type < ObjectDescriptor::e_ShapeItemEnd) {
                ShapeItem *shape_item = static_cast<ShapeItem *>(object);
                emit show_menu_for_shape_item(shape_item);
            }
        } else {
            // menu for property
            emit show_menu_for_property(item->to_property());
        }
    }
}

void CompositionDocument::apply_layer_setting(Layer *layer, const LayerSetting &data)
{
    m_undo_manager->set_layer_setting(layer, data, std::vector<int>{ Qt::DisplayRole});
}

void CompositionDocument::slot_current_frame_changed(int frame)
{
    m_current_frame = frame;
}

void CompositionDocument::update_text_layer_settings(TextLayer *text_layer,
                                                     const core::Text &text)
{
    undo_manager()->update_text_layer_settings(text_layer, text);
}

void CompositionDocument::slot_edited_easing_between(const EasingBetweenTwoFramesData &data)
{
    m_undo_manager->update_easing_between_keyrames(data);
}

void CompositionDocument::emit_document_modified()
{
    emit modified_changed(this);
}

void CompositionDocument::emit_updated_item(const TreeItem *item, std::vector<int> roles)
{
    QList<int> ql_roles;
    ql_roles.reserve(roles.size());
    for (auto &role : roles) {
        ql_roles.append(role);
    }
    updated_item(item, ql_roles);
}

void CompositionDocument::emit_property_added(PropertyGroup *g, Property *p, int i) {
    emit property_added(g, p, i);
}

void CompositionDocument::emit_property_removed(PropertyGroup *g, Property *p, int i) {
    emit property_removed(g, p, i);
}

void CompositionDocument::emit_about_to_insert_layer(Layer *, int index) {
    beginInsertRows(QModelIndex(), index, index);
}

void CompositionDocument::emit_inserted_layer(Layer *layer, int index) {
    endInsertRows();
    emit new_layer_added(layer, index);
}

void CompositionDocument::emit_about_to_remove_layer(Layer * layer, int index) {
    emit about_to_remove_layer(layer);
    beginRemoveRows(QModelIndex(), index, index);
}

void CompositionDocument::emit_removed_layer(Layer *, int) {
    endRemoveRows();
    update_layer_index_info();
}

void CompositionDocument::emit_composition_settings_updated() {
    emit composition_settings_updated();
}

void CompositionDocument::emit_about_to_add_shape_item(ShapeLayer *shape_layer,
                                                       Group *group,
                                                       ShapeItem *shape,
                                                       int shape_index)
{
    const QModelIndex group_model_index = index_of(group);
    beginInsertRows(group_model_index, shape_index, shape_index);
}

void CompositionDocument::emit_shape_item_added(ShapeLayer *shape_layer,
                                                Group *group,
                                                ShapeItem *shape,
                                                int shape_index)
{
    endInsertRows();
    emit shape_added(shape_layer, group, shape, shape_index);
}

int CompositionDocument::emit_about_to_remove_shape(ShapeLayer *shape_layer,
                                                    Group *group,
                                                    ShapeItem *shape,
                                                    int shape_index)
{
    emit about_to_remove_shape(shape_layer, group, shape);
    if (shape_index != -1) {
        const QModelIndex group_model_index = index_of(group);
        beginRemoveRows(group_model_index, shape_index, shape_index);
    }
    return shape_index;
}

void CompositionDocument::emit_shape_removed(int shape_index)
{
    if (shape_index != -1) {
        endRemoveRows();
    }
}

bool CompositionDocument::emit_about_to_move_layer(int source_index, int destination_index)
{
    int q_src = source_index;
    int q_dst = destination_index;

    return beginMoveRows(QModelIndex(), q_src, q_src, QModelIndex(), q_dst);
}

void CompositionDocument::emit_moved_layers(int from, int to)
{
    endMoveRows();
    emit layer_moved(from, to);
}

bool CompositionDocument::emit_about_to_move_shape(ShapeLayer *shape_layer,
                                                   Object *object,
                                                   int source_index,
                                                   int destination_index)
{
    bool result = false;
    const TreeItem *from_grand_parent = object->parent();
    QModelIndex from_parent_index = createIndex(from_grand_parent->index_of_item(object), 0, object);
    if (beginMoveRows(from_parent_index,
                      source_index,
                      source_index,
                      from_parent_index,
                      destination_index)) {
        result = true;
    }
    return result;
}

void CompositionDocument::emit_shape_moved(ShapeLayer *shape_layer,
                                           Object *object,
                                           int source_index,
                                           int destination_index)
{
    endMoveRows();
    emit shapes_moved(shape_layer, object, source_index, destination_index);
}

void CompositionDocument::emit_text_layer_settings_changed(TextLayer *text_layer,
                                                           const core::Text &text)
{
    emit text_layer_settings_changed(text_layer, text);
}

void CompositionDocument::delete_layers(std::vector<Layer *> &layers)
{
    m_undo_manager->remove_layers(layers);
    update_layer_index_info();
}

void CompositionDocument::create_precomp_layer(const std::string &precomp_path)
{
    using namespace commands;

    Vec3D pos_3d;
    pos_3d.x() = m_comp->width() / 2;
    pos_3d.y() = m_comp->height() / 2;
    pos_3d.z() = 0;
    model::PrecompositionLayer *layer = new PrecompositionLayer(nullptr,
                                                                m_new_layer_index++,
                                                                precomp_path);
    layer->set_name(layer_type_name(LayerType::e_Precomp) + "_"
                    + std::to_string(layer->layer_index()));
    layer->set_in_point(m_comp->in_point());
    layer->set_out_point(m_comp->out_point());
    model::Transform &transform = layer->transform();
    transform.set_anchor(pos_3d);
    transform.set_position(pos_3d);
    layer->set_editor_color_index(gui::LayerColorModel::instance()->get_random_color_index());
    m_undo_manager->insert_new_layers(std::vector<Layer*>{layer}, -1);
    update_layer_index_info();
}

void CompositionDocument::import_layers(Layers &layers)
{
    using namespace commands;
    std::vector<Layer *> layers_to_insert;
    for (auto &layer : layers) {
        layers_to_insert.emplace_back(layer.get());
    }
    m_undo_manager->insert_new_layers(layers_to_insert, -1);
    update_layer_index_info();
}

std::string CompositionDocument::playground_script() const
{
    return m_playground_script;
}

void CompositionDocument::set_playground_script(const std::string &script)
{
    if (m_playground_script != script) {
        m_playground_script = script;
        set_modified(true);
    }
}

std::string CompositionDocument::apply_script() const
{
    return m_comp->apply_script();
}

ITagModel *CompositionDocument::tag_model() {
    return m_tag_model;
}

} // namespace alive::model
