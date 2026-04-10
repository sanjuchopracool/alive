#include "layer_setting.h"
#include "file_based_layer_setting.h"
#include "solid_layer_setting.h"
#include "text_layer_setting.h"
#include <core/model/layers/image_layer.h>
#include <core/model/layers/precomposition_layer.h>
#include <core/model/layers/solid_layer.h>
#include <core/model/layers/text_layer.h>

namespace alive::model {
LayerSetting image_layer_setting(const ImageLayer *layer)
{
    FileBasedLayerSetting setting;
    setting.name = layer->name();
    setting.path = layer->path();
    return setting;
}

LayerSetting precomp_layer_setting(const PrecompositionLayer *layer)
{
    FileBasedLayerSetting setting;
    setting.name = layer->name();
    setting.path = layer->precomp_path();
    return setting;
}

void apply_image_layer_setting(ImageLayer *layer, const LayerSetting &setting_data)
{
    const FileBasedLayerSetting *setting = std::get_if<FileBasedLayerSetting>(&setting_data);
    if (setting) {
        layer->set_name(setting->name);
        layer->set_path(setting->path);
    }
}

LayerSetting text_layer_setting(const TextLayer *layer)
{
    TextLayerSetting setting;
    setting.name = layer->name();
    return setting;
}

LayerSetting solid_layer_setting(const SolidLayer *layer)
{
    SolidLayerSetting setting;
    setting.width = layer->width();
    setting.height = layer->height();
    setting.name = layer->name();
    setting.color = layer->color();
    return setting;
}

void apply_solid_layer_setting(SolidLayer *layer, const LayerSetting &setting_data)
{
    const SolidLayerSetting *setting = std::get_if<SolidLayerSetting>(&setting_data);
    if (setting)
    {
        layer->set_width(setting->width);
        layer->set_height(setting->height);
        layer->set_name(setting->name);
        layer->set_color(setting->color);
    }
}

LayerSetting layer_setting(const Layer *layer)
{
    switch (layer->layer_type()) {
    case LayerType::e_Image:
        return image_layer_setting(static_cast<const ImageLayer *>(layer));
    case LayerType::e_Precomp:
        return precomp_layer_setting(static_cast<const PrecompositionLayer *>(layer));
    case LayerType::e_Text:
        return text_layer_setting(static_cast<const TextLayer *>(layer));
    case LayerType::e_Solid:
        return solid_layer_setting(static_cast<const SolidLayer *>(layer));
    default:
        break;
    }
    return {};
}

bool apply_layer_setting(Layer *layer, const LayerSetting &setting_data)
{
    switch (layer->layer_type()) {
    case LayerType::e_Image:
        apply_image_layer_setting(static_cast<ImageLayer *>(layer), setting_data);
        break;
    // case LayerType::e_Text:
    //     return text_layer_setting(static_cast<const TextLayer *>(layer));
    case LayerType::e_Solid:
        apply_solid_layer_setting(static_cast<SolidLayer *>(layer), setting_data);
        break;
    default:
        break;
    }
    return true;
}

} // namespace alive::model
