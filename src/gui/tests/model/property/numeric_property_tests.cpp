#include "numeric_property_tests.h"
#include <QMetaObject>
#include <QQmlContext>
#include <QQmlEngine>
#include <QVariant>

namespace inae::model {
void PrimitivePropertyTests::basic_tests()
{
    Transform obj;
    QCOMPARE(ObjectDescriptor::e_Transform, obj.object_type());
    QCOMPARE(PropertyDescriptor::e_Object, obj.type());

    QCOMPARE(obj.get_rotation(), 0.0f);
    QCOMPARE(obj.get_opacity(), 100.0f);
    obj.set_opacity(23.0f);
    QCOMPARE(obj.get_opacity(), 23.0f);

    //    const QMetaObject &meta_object = Transform::staticMetaObject;
    //    QCOMPARE(obj.property_count(), meta_object.propertyCount());
    //    for (int i = 0; i < meta_object.propertyCount(); ++i) {
    //        QMetaProperty prop = meta_object.property(i);
    //        if (QString("opacity") == prop.name()) {
    //            QCOMPARE(prop.metaType(), QMetaType(QMetaType::Float));
    //        } else if (QString("rotation") == prop.name()) {
    //            QCOMPARE(prop.metaType(), QMetaType(QMetaType::Float));
    //        }
    //    }
}

void PrimitivePropertyTests::script_test()
{
    //    qRegisterMetaType<Transform>();
    QJSEngine engine;
    TransformObj obj;
    obj.setObjectName("CHECK");
    qDebug() << obj.get_transform();
    //    obj.get_transform()->set_opacity(23.23f);
    auto js_obj = engine.newQObject(&obj);
    engine.globalObject().setProperty("obj", js_obj);
    qDebug() << engine.evaluate("JSON.stringify(obj)").toString();
    //    qDebug() << engine.evaluate("JSON.stringify(obj.transform)").toString();
    //    qDebug() << engine.evaluate("obj.transform").toString();
    qDebug() << engine.evaluate("var val = obj.transform.opacity; val").toNumber();
    qDebug() << obj.get_transform()->get_opacity();

    //    qRegisterMetaType<Transform>("NumericObject");
    //    QCOMPARE(true, engine.evaluate("obj.transform.opacity = 12.23").toBool());
    //    QCOMPARE(obj.get_transform()->get_opacity(), 12.23f);
}

TransformObj::TransformObj(QObject *parent)
    : QObject(parent)
{}

} // namespace inae::model
