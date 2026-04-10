#ifndef PROPERTY_WIDGET_FACTORY_H
#define PROPERTY_WIDGET_FACTORY_H

class QWidget;
namespace alive {
namespace model {
class Property;
}
namespace property {
class PropertyWidgetFactory
{
public:
    static QWidget *widget_for_property(alive::model::Property *property);
};
} // namespace property
} // namespace alive
#endif // PROPERTY_WIDGET_FACTORY_H
