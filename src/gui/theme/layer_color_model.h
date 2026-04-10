#ifndef EDITOR_LAYER_COLORS_TEMPLATE_H
#define EDITOR_LAYER_COLORS_TEMPLATE_H

#include <memory>
#include <QAbstractListModel>
#include <QColor>
#include <QObject>
#include <QString>

QT_BEGIN_NAMESPACE
class QQmlEngine;
class QJSEngine;
QT_END_NAMESPACE

namespace alive::gui {
struct LayerColorsData;
struct EditorLayerColorValue
{
    Q_GADGET
    Q_PROPERTY(QString name MEMBER name CONSTANT)
    Q_PROPERTY(QColor color MEMBER color CONSTANT)
public:
    QString name = "Invalid";
    QColor color;
};

class LayerColorModel : public QAbstractListModel
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(LayerColorModel)
public:
    ~LayerColorModel();
    // QAbstractListModel
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    static QObject *qml_instance(QQmlEngine *engine, QJSEngine *script_engine)
    {
        Q_UNUSED(engine);
        Q_UNUSED(script_engine);
        return instance();
    }

    static void init();
    static void deinit(bool qml = false);
    static LayerColorModel *instance();

    Q_INVOKABLE EditorLayerColorValue editor_color_at_index(int index);

    int count() const;
    int get_random_color_index() const;
    int unset_index() const { return 0; };

private:
    explicit LayerColorModel(QObject *parent = nullptr);

private:
    std::unique_ptr<LayerColorsData> m_d;
};

EditorLayerColorValue editor_color_at_index(int index);
int editor_color_max_index();
} // namespace alive::gui

Q_DECLARE_METATYPE(alive::gui::EditorLayerColorValue)

#endif // EDITOR_LAYER_COLORS_TEMPLATE_H
