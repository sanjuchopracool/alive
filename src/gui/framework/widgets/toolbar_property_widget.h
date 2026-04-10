#ifndef TOOLBAR_PROPERTY_WIDGET_H
#define TOOLBAR_PROPERTY_WIDGET_H

#include <Corrade/Containers/Pointer.h>
#include <QTabWidget>
#include <QWidget>

namespace alive {

namespace model {
class PropertyGroup;
class Property;
enum class PropertyType;
class CompositionDocument;
}

class ToolBar;
class ToolButton;
class TextPropertyWidget;
class TagModel;

namespace core {
class Text;
}

enum class ToolBarContextType { e_None = 0b0, e_Text = 0b1, e_Property = 0b10, e_Tagging = 0b100 };

struct ToolBarPropertyWidgetPrivateData;
class ToolBarPropertyWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ToolBarPropertyWidget(QWidget *parent = nullptr);
    ~ToolBarPropertyWidget();

    int expanded_width() const;
    void update_expanded_width();

    // text related
    void enable_text(bool enable);
    void set_text_properties(const core::Text &text);

    // property groupœ
    void set_property_group(model::CompositionDocument *doc,
                            model::PropertyGroup *group,
                            bool enable_editing);

    void update_current_widget();
    void set_tag_model(TagModel *model);

public:
    void restore_settings();
    void save_settings() const;

signals:
    void toolbar_visibility_changed(int);
    void text_changed(const core::Text &text);

private:
    Corrade::Containers::Pointer<ToolBarPropertyWidgetPrivateData> m_d;
};
} // namespace alive

#endif // TOOLBAR_PROPERTY_WIDGET_H
