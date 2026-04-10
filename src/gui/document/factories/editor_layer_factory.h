#ifndef EDITORLAYERFACTORY_H
#define EDITORLAYERFACTORY_H

#include <memory>
#include <editor/settings/layer_setting.h>

namespace alive::model {
class Layer;
class Composition;
enum class LayerType;
} // namespace alive::model
namespace alive::EditorLayerFactory {

using CreateInfo = std::pair<model::LayerType, int>;
using LayerData = std::pair<std::unique_ptr<model::Layer>, model::LayerSetting>;

LayerData create_layer(CreateInfo info, const model::Composition &comp);

} // namespace alive::EditorLayerFactory

#endif // EDITORLAYERFACTORY_H
