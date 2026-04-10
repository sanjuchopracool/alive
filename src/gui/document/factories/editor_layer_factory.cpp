#include "editor_layer_factory.h"
#include <core/alive_types/common_types.h>
#include <core/model/composition.h>
#include <core/model/layers/image_layer.h>
#include <core/model/layers/null_layer.h>
#include <core/model/layers/precomposition_layer.h>
#include <core/model/layers/shape_layer.h>
#include <core/model/layers/solid_layer.h>
#include <core/model/layers/text_layer.h>
#include <gui/theme/layer_color_model.h>
#include <string>

namespace alive::EditorLayerFactory {

LayerData create_layer(CreateInfo info, const model::Composition &comp)
{
    using namespace model;
    auto [type, index] = info;
    LayerData result;
    auto &[layerPtr, setting] = result;

    Vec3D pos_3d;
    pos_3d.x() = comp.width() / 2;
    pos_3d.y() = comp.height() / 2;
    pos_3d.z() = 0;

    switch (type) {
    case model::LayerType::e_Text: {
        auto text_layer = std::make_unique<model::TextLayer>(nullptr, index);
        layerPtr.reset(text_layer.release());
    } break;
    case model::LayerType::e_Image: {
        auto image_layer = std::make_unique<ImageLayer>(nullptr, index);
        layerPtr.reset(image_layer.release());
    } break;
    case model::LayerType::e_Precomp: {
        auto precomp_layer = std::make_unique<PrecompositionLayer>(nullptr, index);
        precomp_layer->transform().set_anchor(pos_3d);
        layerPtr.reset(precomp_layer.release());
    } break;
    case model::LayerType::e_Shape: {
        auto shape_layer = std::make_unique<ShapeLayer>(nullptr, index);
        layerPtr.reset(shape_layer.release());
    } break;
    case model::LayerType::e_Null: {
        auto null_layer = std::make_unique<NullLayer>(nullptr, index);
        layerPtr.reset(null_layer.release());
    } break;
    case model::LayerType::e_Solid: {
        auto solid_layer = std::make_unique<SolidLayer>(nullptr, index);
        solid_layer->set_width(comp.width());
        solid_layer->set_height(comp.height());
        solid_layer->set_color(Color4ub::fromLinearRgbaInt(0xFF));
        solid_layer->transform().set_anchor(pos_3d);
        layerPtr.reset(solid_layer.release());
    } break;
    default:
        break;
    }

    if (layerPtr) {
        Layer *layer = layerPtr.get();
        setting = model::layer_setting(layer);
        layer->set_name(layer_type_name(type) + "_" + std::to_string(index));
        layer->set_in_point(comp.in_point());
        layer->set_out_point(comp.out_point());
        model::Transform &transform = layer->transform();
        transform.set_position(pos_3d);
        layer->set_editor_color_index(gui::LayerColorModel::instance()->get_random_color_index());
    }
    return result;
}

} // namespace alive::EditorLayerFactory
