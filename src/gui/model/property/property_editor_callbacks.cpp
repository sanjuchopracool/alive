#include "property_editor_callbacks.h"
#include <core/model/property/dynamic_property.h>
#include <core/model/property/property_type.h>
#include <editor/undo_support/property_commands.h>
#include <gui/document/composition_document.h>
#include <QRegularExpression>

namespace alive::gui {

template<typename T>
const DynamicProperty<T> *const_dynamic_property(const Property *property)
{
    return static_cast<const DynamicProperty<T> *>(property);
}

PropertyTextValue text_for_property(const Property *property, int current_frame, int decimal_point)
{
    auto to_number = [decimal_point](float val) {
        QString str = QString::number(val, 'f', decimal_point);
        static QRegularExpression p1("0+$");  // Remove any number of trailing 0's
        static QRegularExpression p2("\\.$"); // If the last character is just a '.' then remove it
        str.remove(p1);
        str.remove(p2);
        return str;
    };

    PropertyTextValue result;
    switch (property->type()) {
    case alive::model::PropertyType::e_NumPoints: {
        const auto *d_prop = const_dynamic_property<Vec1D>(property);
        result.t1 = to_number(static_cast<int>(d_prop->get_value()));
        result.count = 1;
    } break;
    case PropertyType::e_Opacity:
    case PropertyType::e_Roundness:
    case PropertyType::e_Float: {
        const auto *d_prop = const_dynamic_property<float>(property);
        result.t1 = to_number(d_prop->get_value());
        result.count = 1;
    } break;
    case PropertyType::e_Rotation: {
        const auto *d_prop = const_dynamic_property<float>(property);
        float val = d_prop->get_value();
        int rotation = static_cast<int>(val / 360);
        result.t1 = QString::number(rotation);
        result.t2 = to_number(val - rotation * 360);
        if (val > 0) {
            result.t2.prepend('+');
        }
        result.count = 2;
    } break;
    case PropertyType::e_Anchor3D:
    case PropertyType::e_Position3D:
    case PropertyType::e_Scale3D: {
        const auto *d_prop = const_dynamic_property<Vec3D>(property);
        result.t1 = to_number(d_prop->get_value().x());
        result.t2 = to_number(d_prop->get_value().y());
        result.count = 2;
    } break;
    case PropertyType::e_Position2D:
    case model::PropertyType::e_Anchor2D:
    case model::PropertyType::e_Scale2D:
    case PropertyType::e_Size2D: {
        const auto *d_prop = const_dynamic_property<Vec2D>(property);
        result.t1 = to_number(d_prop->get_value().x());
        result.t2 = to_number(d_prop->get_value().y());
        result.count = 2;
    } break;
    default:
        break;
    }

    switch (property->type()) {
    case PropertyType::e_Opacity:
    case PropertyType::e_Scale3D:
    case PropertyType::e_Roundness:
        result.suffix = "%";
        break;
    case PropertyType::e_Rotation:
        result.suffix = "°";
        result.seperator = "x";
        break;
    default:
        break;
    }

    //    qDebug() << result.t1 << result.t2;
    return result;
}

void update_property_change(
    Property *property, KeyFrameTime time, EditingField field, Document *document, int change)
{
    Q_ASSERT(field != EditingField::e_None);
    std::vector<int> roles
        {CompositionDocument::IsAnimating , CompositionDocument::KeyFrameCount , CompositionDocument::EditValue};
    document->undo_manager()->update_property_change(property, time, field, change, roles);
}

bool update_property_change(Property *property,
                            KeyFrameTime time,
                            EditingField field,
                            Document *document,
                            const QString &new_value_str)
{
    Q_ASSERT(field != EditingField::e_None);
    std::vector<int> roles
        {CompositionDocument::IsAnimating , CompositionDocument::KeyFrameCount , CompositionDocument::EditValue};
    return document->undo_manager()->update_property_change(property, time, field, new_value_str.toStdString(), roles);
}

} // namespace alive::gui
