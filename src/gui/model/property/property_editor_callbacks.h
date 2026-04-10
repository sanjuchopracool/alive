#ifndef PROPERTYEDITORCALLBACKS_H
#define PROPERTYEDITORCALLBACKS_H

#include <core/alive_types/common_types.h>
#include <QObject>
#include <QString>

namespace alive {
class Document;

namespace model {
class Property;
}
} // namespace alive::model
namespace alive::gui {
using namespace model;

struct PropertyTextValue
{
public:
    QString t1;
    QString t2;
    QString seperator = ",";
    QString suffix;
    int count = 0;
};

PropertyTextValue text_for_property(const Property *property,
                                    int current_frame,
                                    int decimal_point = 1);
void update_property_change(Property *property,
                            KeyFrameTime frame,
                            EditingField field,
                            Document *document,
                            int change);
bool update_property_change(Property *property,
                            KeyFrameTime frame,
                            EditingField field,
                            Document *document,
                            const QString &new_value_str);
} // namespace alive::gui

#endif // PROPERTYEDITORCALLBACKS_H
