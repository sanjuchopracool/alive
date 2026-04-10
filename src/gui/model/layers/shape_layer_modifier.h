#ifndef SHAPELAYERMODIFIER_H
#define SHAPELAYERMODIFIER_H

#include <QObject>
#include <QVariantList>

QT_BEGIN_NAMESPACE
class QQmlEngine;
class QJSEngine;
QT_END_NAMESPACE

namespace alive::gui {
struct ContentGroupItem
{
    Q_GADGET
    Q_PROPERTY(QString name MEMBER name CONSTANT)
    Q_PROPERTY(int data MEMBER data CONSTANT)
public:
    QString name;
    int data;
};

class ShapeLayerModifier : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(ShapeLayerModifier)

public:
    ~ShapeLayerModifier();
    static QObject *qml_instance(QQmlEngine *engine, QJSEngine *script_engine)
    {
        Q_UNUSED(engine);
        Q_UNUSED(script_engine);
        return instance();
    }

    static ShapeLayerModifier *instance()
    {
        static ShapeLayerModifier *the_instance = nullptr;
        if (!the_instance) {
            the_instance = new ShapeLayerModifier();
        }
        return the_instance;
    }

    Q_INVOKABLE QVariantList shape_objects() const;

private:
    ShapeLayerModifier();
};
} // namespace alive::gui
Q_DECLARE_METATYPE(alive::gui::ContentGroupItem)

#endif // SHAPELAYERMODIFIER_H
