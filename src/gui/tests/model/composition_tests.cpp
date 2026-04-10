#include "composition_tests.h"
#include <color_type.h>
#include <composition/composition_node.h>
#include <composition/solid_layer_node.h>
#include <model/composition.h>
#include <model/layers/solid_layer.h>
#include <serializer/inae_serializer.h>
#include <QFile>
#include <QJsonDocument>
#include <QStringList>

namespace inae::model {

CompositionTests::CompositionTests(QObject *parent)
    : QObject{parent}
{
    
}

void CompositionTests::parse_test()
{
    QFile file(":/SOLID_LAYER.json");
    if (file.open(QIODevice::ReadOnly)) {
        inae::core::SerializerWarnings messages;
        const QByteArray data = file.readAll();
        JsonObject obj = JsonObject::parse(data);
        std::unique_ptr<Composition> comp = inae::gui::InaeSerializer::parse_inae(obj, messages);
        for (const auto &message : messages)
            qDebug() << message;

        QCOMPARE(comp->property_count(), 2);
        QCOMPARE(comp->type(), PropertyDescriptor::e_Object);
        QCOMPARE(comp->object_type(), ObjectDescriptor::e_Composition);

        for (int i = 0; i < comp->property_count(); ++i) {
            Property *prop_layer = comp->property_at(i);
            QCOMPARE(prop_layer->type(), PropertyDescriptor::e_Object);
            QCOMPARE(prop_layer->property_name(), std::string("SolidLayer"));
            Object *object_layer = static_cast<Object *>(prop_layer);
            QCOMPARE(object_layer->object_type(), ObjectDescriptor::e_Layer);
            Layer *layer_obj = static_cast<Layer *>(object_layer);
            QCOMPARE(layer_obj->layer_type(), LayerType::e_Solid);
            QCOMPARE(layer_obj->property_count(), 1);
            for (int j = 0; j < layer_obj->property_count(); ++j) {
                Property *prop_transform = layer_obj->property_at(j);
                qDebug() << prop_transform->property_name();
                QCOMPARE(prop_transform->type(), PropertyDescriptor::e_Object);
                Object *object_transform = static_cast<Object *>(prop_transform);
                QCOMPARE(object_transform->object_type(), ObjectDescriptor::e_Transform);
                QCOMPARE(object_transform->property_count(), 5);
            }
        }

        CompositionNode comp_node(comp.get());
        const SolidLayerNode *red_solid_layer = comp_node.get_solid_layer("Red Solid 1");
        const SolidLayerNode *royal_blue_solid_layer = comp_node.get_solid_layer(
            "Royal Blue Solid 1");
        QCOMPARE_NE(red_solid_layer, nullptr);
        QCOMPARE_NE(royal_blue_solid_layer, nullptr);
        inae::script::ExpressionEngine &engine = comp_node.expr_engin();
        const SkColor &red_color = engine.evaulate(
            "return comp:get_solid_layer(\"Red Solid 1\"):color()");
        qDebug() << skcolor_to_rgb_string(red_color);
        QCOMPARE(red_color, red_solid_layer->color());
        int red_width = engine.evaulate("return comp:get_solid_layer(\"Red Solid 1\"):width()");
        qDebug() << red_width;
        QCOMPARE(red_width, red_solid_layer->width());
        //run bad script
        auto result = engine.evaulate_safely(
            "return comp:get_solid_layer(\"NOT_EXISTING_LAYER\"):width()");
        QCOMPARE(result.valid(), false);

        result = engine.evaulate_safely("return comp:get_solid_layer(\"Red Solid 1\").width()");
        QCOMPARE(result.valid(), false);
    }
}
} // namespace inae::model
