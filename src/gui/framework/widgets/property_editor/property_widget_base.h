#ifndef PROPERTY_WIDGET_BASE_H
#define PROPERTY_WIDGET_BASE_H

#include <QWidget>

QT_FORWARD_DECLARE_CLASS(QVBoxLayout)

namespace alive {
namespace model {
class Property;
}
namespace property {
class PropertyWidgetBase : public QWidget
{
    Q_OBJECT
public:
    PropertyWidgetBase(model::Property *prop,
                       bool enable_name_editing = false,
                       QWidget *parent = nullptr);
    ~PropertyWidgetBase();

signals:
    void delete_clicked(model::Property *prop);
    void duplicate_clicked(model::Property *prop);
    void up_clicked(model::Property *prop);

private:
    model::Property *m_property;
    QVBoxLayout *m_layout = nullptr;
};
} // namespace property
} // namespace alive

#endif // PROPERTY_WIDGET_BASE_H
