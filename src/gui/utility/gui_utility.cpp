#include "gui_utility.h"
#include <core/model/layers/layer.h>
#include <core/model/property/dynamic_property.h>
#include <core/model/property/object.h>
#include <core/model/shape_items/rectangle.h>
#include <core/model/shape_items/shape_item.h>
#include <editor/undo_support/document_undo_manager.h>
#include <editor/undo_support/property_commands.h>
#include <editor/undo_support/shape_item_commands.h>
#include <gui/color_utility.h>
#include <gui/font_model.h>
#include <gui/model/property/property_editor_callbacks.h>
#include <gui/document/composition_document.h>
#include <QFont>

namespace alive::gui {

using namespace model;
Utility::Utility(QObject *parent)
    : QObject{parent}
{
    m_blend_modes = {
        "Normal",
        "Multiply",
        "Screen",
        "Overlay",
        "Darken",
        "Lighten",
        "ColorDodge",
        "ColorBurn",
        "HardLight",
        "SoftLight",
        "Difference",
        "Exclusion",
        "Hue",
        "Saturation",
        "Color",
        "Luminosity",
        "Add",
        "HardMix",
    };

    m_matte_modes = {"None", "Alpha", "Alpha Inverted", "Luma", "Luma Inverted"};

    Q_ASSERT(m_blend_modes.size() == static_cast<int>(BlendMode::e_Last));
    Q_ASSERT(m_matte_modes.size() == static_cast<int>(MatteType::e_Last));

    connect(FontModel::instance(), &FontModel::current_index_changed, this, [this]() {
        if (m_font_setting.m_family != FontModel::instance()->current_index()) {
            m_font_setting.m_family = FontModel::instance()->current_index();
            emit font_setting_changed();
        }
    });

    connect(FontModel::instance(), &FontModel::style_index_changed, this, [this]() {
        if (m_font_setting.m_style != FontModel::instance()->style_index()) {
            m_font_setting.m_style = FontModel::instance()->style_index();
            emit font_setting_changed();
        }
    });
    m_font_setting.m_style = FontModel::instance()->style_index();
    m_font_setting.m_family = FontModel::instance()->current_index();
}

template<typename T>
const T &dynamic_property_value(const Property *property)
{
    const DynamicProperty<T> *d_prop = static_cast<const DynamicProperty<T> *>(property);
    return d_prop->get_value();
}

template<typename T>
bool get_path_direction(const Object *object)
{
    const T *item = static_cast<const T *>(object);
    return !(item->path_direction() == PathDirection::e_CounterClockwise);
}

namespace {
static Utility *the_instance = nullptr;
}

void Utility::init()
{
    if (!the_instance) {
        the_instance = new Utility();
    }
}

void Utility::deinit(bool qml)
{
    if (!qml && the_instance) {
        delete the_instance;
        the_instance = nullptr;
    }
}

Utility *Utility::instance()
{
    return the_instance;
}

QVariant Utility::edit_value(const Property *property,
                             const Object *object,
                             const Layer *layer) const
{
    EditColumnValue edit_value;
    if (layer) {
        edit_value.m_type = EditValueType::LayerProperties;
        QVariantList infos;
        infos.resize(static_cast<int>(EditColumnValue::PropLast));
        infos[EditColumnValue::BlendMode] = (static_cast<int>(layer->blend_mode()));
        infos[EditColumnValue::MatteType] = (static_cast<int>(layer->matte_type()));
        infos[EditColumnValue::MatteLayerIndex] = layer->matte_layer_index();
        infos[EditColumnValue::ParentLayerIndex] = layer->parent_layer_index();
        edit_value.m_value.setValue(infos);
    } else if (object) {
        switch (object->object_type()) {
        case ObjectDescriptor::e_ContentGroup:
            edit_value.m_type = EditValueType::ShapeContent;
            break;
        case ObjectDescriptor::e_Group:
            edit_value.m_type = EditValueType::ShapeGroup;
            break;
        case ObjectDescriptor::e_RectanglePath:
        case ObjectDescriptor::e_EllipsePath:
        case ObjectDescriptor::e_PolyStarPath:
            edit_value.m_type = EditValueType::PathDirection;
            edit_value.m_value = get_path_direction<model::PathShapeItem>(object);
            break;
        default:
            break;
        }
    } else {
        switch (property->type()) {
        case PropertyType::e_Color: {
            edit_value.m_type = EditValueType::Color;
            Vec3D value = dynamic_property_value<Vec3D>(property);
            edit_value.m_value.setValue(qcolor_from_vec3d(value));
            //            qDebug() << "Read" << edit_value.m_value;
        } break;
        default: {
            edit_value.m_type = EditValueType::PropertyText;
            edit_value.m_value.setValue(text_for_property(property, 0));
            //            qDebug() << "Read" << edit_value.m_value;
        } break;
        }
    }

    QVariant var;
    var.setValue(edit_value);
    return var;
}

bool Utility::set_edit_value(model::CompositionDocument *document,
                             model::Property *property,
                             model::Object *object,
                             model::Layer *layer,
                             KeyFrameTime time,
                             QVariant var)
{
    bool result = false;
    EditColumnValue edit_value = var.value<EditColumnValue>();
    if (layer) {
        if (edit_value.m_type == EditValueType::LayerProperties) {
            QVariantList infos = edit_value.m_value.toList();
            Q_ASSERT(infos.size() == static_cast<int>(EditColumnValue::PropLast));
            layer->set_blend_mode(static_cast<BlendMode>(infos[EditColumnValue::BlendMode].toInt()));
            layer->set_matte_type(static_cast<MatteType>(infos[EditColumnValue::MatteType].toInt()));
            layer->set_matte_layer_index(infos[EditColumnValue::MatteLayerIndex].toInt());
        }
    } else if (object) {
        switch (object->object_type()) {
        case ObjectDescriptor::e_RectanglePath:
        case ObjectDescriptor::e_PolyStarPath:
        case ObjectDescriptor::e_EllipsePath: {
            PathShapeItem *path_item = static_cast<PathShapeItem *>(object);
            std::vector<int> roles{CompositionDocument::EditValue};
            return document->undo_manager()->set_path_direction(path_item,
                                                                edit_value.m_value.toBool(),
                                                                roles);
        } break;
        default:
            break;
        }
    } else {
        switch (edit_value.m_type) {
            //        case PropertyType::e_Float:
            //            edit_value.m_type = EditValueType::Vec1D;
            //            break;
        case EditValueType::Color: {
            edit_value.m_type = EditValueType::Color;
            QColor color = edit_value.m_value.value<QColor>();
            //            qDebug() << "Write" << color;
            Vec3D value = qcolor_to_vec3d(color);
            std::vector<int> roles =
            {CompositionDocument::IsAnimating,CompositionDocument::KeyFrameCount, CompositionDocument::EditValue};
            return document->undo_manager()->change_dynamic_property_value<Vec3D>(property, time , value, roles);
            }
            break;
        default:
            break;
        }
    }
    return result;
}

const QStringList &Utility::blend_modes() const
{
    return m_blend_modes;
}

const QStringList &Utility::matte_modes() const
{
    return m_matte_modes;
}

void Utility::set_text_setting(model::SourceTextSetting new_setting)
{
    if (m_text_setting != new_setting) {
        m_text_setting = new_setting;
        emit text_setting_changed();
    }
}

void Utility::set_stroke_setting(model::StrokeSetting &new_setting)
{
    if (m_stroke_setting != new_setting) {
        m_stroke_setting = new_setting;
        emit stroke_setting_changed();
    }
}
bool Utility::validate_object_name(const QString &object_name)
{
    for (int i = 0; i < object_name.size(); ++i) {
        QChar ch = object_name[i];
        if (!(ch.isLetterOrNumber() || ch == '_' || ch == '-')) {
            return false;
        }
    }
    return true;
}

model::FillSetting Utility::fill_setting() const
{
    return m_fill_setting;
}

void Utility::set_fill_setting(const model::FillSetting &new_setting)
{
    if (m_fill_setting != new_setting) {
        m_fill_setting = new_setting;
        emit fill_setting_changed();
    }
}

model::FontSetting Utility::font_setting() const
{
    return m_font_setting;
}

void Utility::set_font_setting(const model::FontSetting &new_setting)
{
    if (m_font_setting != new_setting) {
        m_font_setting = new_setting;
        emit font_setting_changed();
    }
}

void Utility::update_font_setting(const model::FontSetting &new_setting)
{
    if (m_font_setting != new_setting) {
        m_font_setting = new_setting;
        FontModel::instance()->set_current_index(new_setting.m_family);
        FontModel::instance()->set_style_index(new_setting.m_style);
        emit font_setting_changed();
    }
}

const QString &Utility::matte_mode(int index) const
{
    // return none
    if (index >= m_matte_modes.size() || index < 0) {
        index = 0;
    }

    return m_matte_modes[index];
}

} // namespace alive::gui
