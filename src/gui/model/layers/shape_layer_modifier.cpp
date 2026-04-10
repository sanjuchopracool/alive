#include "shape_layer_modifier.h"
#include <core/model/shape_items/shape_item.h>

namespace alive::gui {

ShapeLayerModifier::~ShapeLayerModifier() {}

QVariantList ShapeLayerModifier::shape_objects() const
{
    using namespace model;
    static QVariantList data_model;
    if (data_model.isEmpty()) {
        for (int i = static_cast<int>(ShapeType::e_None) + 1;
             i < static_cast<int>(ShapeType::e_Last);
             ++i) {
            QVariant data;
            ContentGroupItem item;
            item.data = i;
            item.name = QString::fromStdString(shape_type_to_name(static_cast<ShapeType>(i)));
            data.setValue(item);
            data_model.emplaceBack(data);
        }
    }
    return data_model;
}

ShapeLayerModifier::ShapeLayerModifier() {}
} // namespace alive::gui
