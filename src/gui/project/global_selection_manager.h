#ifndef GLOBAL_SELECTION_MANAGER_H
#define GLOBAL_SELECTION_MANAGER_H

#include <Corrade/Containers/Pointer.h>
#include <QObject>

namespace alive {

namespace core {
class Text;
}

class ToolBarPropertyWidget;
namespace model {
class Layer;
class TextLayer;
}

struct GlobalSelectionManagerPrivateData;
class GlobalSelectionManager : public QObject
{
    Q_OBJECT
public:
    GlobalSelectionManager(ToolBarPropertyWidget *toolbar, QObject *parent = nullptr);
    ~GlobalSelectionManager();

public slots:
    void on_layer_selected(model::Layer *layer);

private slots:
    void text_layer_settings_changed(model::TextLayer *text_layer,
                                     const core::Text &text);

private:
    void update_toolbars();

private:
    Corrade::Containers::Pointer<GlobalSelectionManagerPrivateData> m_d;
};

} // namespace alive
#endif // GLOBAL_SELECTION_MANAGER_H
