#ifndef LAYER_SETTING_H
#define LAYER_SETTING_H

#include <variant>

#include <editor/settings/file_based_layer_setting.h>
#include <editor/settings/text_layer_setting.h>
#include <editor/settings/solid_layer_setting.h>

namespace alive::model {
class Layer;

using LayerSetting = std::variant<SolidLayerSetting, FileBasedLayerSetting, TextLayerSetting>;
LayerSetting layer_setting(const Layer *layer);
bool apply_layer_setting(Layer *layer, const LayerSetting &setting_data);

}; // namespace alive::model

#endif // LAYER_SETTING_H
