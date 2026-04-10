#ifndef PROPERTY_GROUP_WIDGET_H
#define PROPERTY_GROUP_WIDGET_H

#include <QWidget>

QT_FORWARD_DECLARE_CLASS(QListWidget);
QT_FORWARD_DECLARE_CLASS(QPushButton);
QT_FORWARD_DECLARE_CLASS(QToolButton);

namespace alive {
namespace model {
class PropertyGroup;
class Property;
class CompositionDocument;
enum class PropertyType;
}
class PropertyGroupWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PropertyGroupWidget(QWidget *parent = nullptr);

public:
    void populate(alive::model::CompositionDocument *doc,
                  model::PropertyGroup *group,
                  bool enable_editing);

private slots:
    void on_property_delete(model::PropertyGroup *group, model::Property *prop, int index);
    void on_property_added(model::PropertyGroup *group, model::Property *prop, int index);

private:
    void add_editor_for_property(model::Property *prop, int index);

private:
    QListWidget *m_view = nullptr;
    bool m_enable_editing = false;

    alive::model::CompositionDocument *m_doc = nullptr;
    model::PropertyGroup *m_group = nullptr;
    QPushButton *m_update_script_button;
    QToolButton *m_add_new_property;
};
} // namespace alive
#endif // PROPERTY_GROUP_WIDGET_H
