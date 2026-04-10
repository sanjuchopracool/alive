#ifndef GUI_PROPERTY_H
#define GUI_PROPERTY_H

#include <QObject>
#include <QVariant>

namespace alive::model {

struct EditColumnValue
{
    Q_GADGET
    Q_PROPERTY(ValueType type MEMBER m_type CONSTANT)
    Q_PROPERTY(QVariant value MEMBER m_value)
public:
    // NOLINTBEGIN
    enum ValueType {
        None,
        Int,
        Int2D,
        Int3D,
        Vec1D,
        Vec2D,
        Vec3D,
        Color,
        String,
        ShapeGroup,
        ShapeContent,
        PathDirection,
        PropertyText,
        LayerProperties
    };

    enum LayerPropIndex { BlendMode, MatteType, MatteLayerIndex, ParentLayerIndex, PropLast };
    // NOLINTEND

    Q_ENUM(ValueType)
    Q_ENUM(LayerPropIndex)
    EditColumnValue() {}

    ValueType m_type = None;
    QVariant m_value;
};
using EditValueType = EditColumnValue::ValueType;

} // namespace alive::model
Q_DECLARE_METATYPE(alive::model::EditColumnValue)

#endif // GUI_PROPERTY_H
