#ifndef DOCUMENTMODEL_H
#define DOCUMENTMODEL_H

#include "gui/composition/timeline.h"
#include "gui/document/factories/editor_layer_factory.h"
#include <core/model/property/property.h>
#include <core/model/property/update_listener.h>
#include <core/model/shape_items/shape_item.h>
#include <gui/model/composition_item_model.h>
#include <editor/settings/composition_setting.h>
#include <gui/model/undostack_model.h>
#include <gui/tagging/tag_model.h>
#include <memory.h>
#include <QUuid>

QT_FORWARD_DECLARE_CLASS(QUndoStack)

namespace alive::gui {
class Timeline;
}

namespace alive::commands {
class AliveCommand;
}

namespace alive::core {
class Text;
}

namespace alive {
class CompositionDocumentProxyModel;
class DocumentUndoManager;
}

namespace alive::model {
class Property;
class Layer;
class ShapeLayer;
class EditorShapeLayer;
class UndoStackModel;
class CompositionNode;
class SolidLayerSetting;
class FileBasedLayerSetting;
class EasingBetweenTwoFramesData;
class TextLayer;
class Precomposition;
class PropertyGroup;
using LayerPtr = Corrade::Containers::Pointer<alive::model::Layer>;
using Layers = std::vector<LayerPtr>;

class CompositionDocument : public CompositionItemModel, public SimpleObserver
{
    Q_OBJECT
public:
    // NOLINTBEGIN
    enum CustomRoles {
        IsLayer = Qt::UserRole + 1,
        IsObject,
        LayerColorIndex,
        LayerLocked,
        LayerBlink,
        LabelText,
        LayerColor,
        LayerInOutRange,
        SetInPoint,
        SetOutPoint,
        MoveInOutPoint,
        SetBlendMode,
        SetMatteType,
        SetMatteLayer,
        SetParentLayer,
        ChangeValueT1,
        ChangeValueT2,
        SetValueTextT1,
        SetValueTextT2,
        // object specific
        ObjectSupportVisibility,
        ObjectVisible,
        SetName,
        // property specific
        ItemRole,
        CanAnimate,
        IsAnimating,
        ExprEnabled,
        KeyFrameCount,
        KeyFrameStatus,
        EditValue,
        AddKeyFrame,
        RemoveKeyFrame,
        RemoveAllKeyFrame,
        IsLinkable,
        IsLinked,
        SetLinked,
        // ShapeLayer
        AddShapeItem,
        // DragSupport
        DragSupportStatus,
        // Delete
        CanDelete,
    };

    enum EditingField {
        None,
        First,
        Second,
    };

    enum ValueType {
        Set,
        Change,
    };

    // NOLINTEND
    Q_ENUM(CustomRoles)
    Q_ENUM(EditingField)
    Q_ENUM(ValueType)

    //commands
    void add_new_layers_command(const std::vector<Layer *> &layers, int index = 0);
    void add_remove_layer_command(Layer *layer);

    // property related
    void updated_index(const QModelIndex &index);
    void updated_item(const TreeItem *property, QList<int> roles = QList<int>());
    QModelIndex updated_property_role(const TreeItem *property, int role);
    void set_layer_in_out_points(Layer *layer, KeyFrameTime in_point, KeyFrameTime out_point);

    TagModel *real_tag_model() const;

    void on_update(ISubject *) override;

    // QAbstractItemModel
    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    // properties
    QString name() const;
    void set_name(const QString &name);
    QUuid id() const;
    void set_id(const QUuid &id);
    void container_nodes_updated();

    // keyframes related
    void move_keyframes(const std::set<model::KeyFrame *> &keyframes, KeyFrameTime t);

    void apply_layer_setting(Layer *layer, const model::LayerSetting &data);

    // shape layer

    std::pair<Layer *, LayerSetting> create_and_push_new_layer(model::LayerType type);
    std::pair<Layer *, LayerSetting> create_new_layer(model::LayerType type);

    // selection

    QVariant get_property_keyframe_status(const Property *property, KeyFrameTime time) const;
    void property_remove_keyframe_at(Property *item, KeyFrameTime time);
    void property_add_keyframe_at(Property *property, KeyFrameTime time);
    void property_remove_all_keyframes(Property *property);
    void invoke_context_menu_for_property(TreeItem *item);

    ~CompositionDocument();

    virtual alive::CompositionDocumentProxyModel *proxy_model();

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    void move_items(TreeItem *from, TreeItem *to, bool above);
    const QString &layer_name(int layer_index) const;
    const std::map<int, QString> &layer_index_to_name_map() const;
    void set_show_matte_details(bool show);
    bool show_matte_details() const;

    // delete item
    void delete_item(TreeItem *item);
    // clone
    void clone_items(std::set<TreeItem *> items);
    // command realted code
    void delete_layers(std::vector<model::Layer *> &layers);

    void create_precomp_layer(const std::string &precomp_path);

    // import
    void import_layers(Layers &layers);

    std::string playground_script() const;
    void set_playground_script(const std::string &script);

    std::string apply_script() const;
    ITagModel* tag_model() override;
    bool is_applying_script() {return m_is_applying_script;}

protected:
    CompositionDocument(Composition *comp, QObject *parent = nullptr);

public slots:
    void slot_current_frame_changed(int frame);
    void update_text_layer_settings(model::TextLayer *text_layer,
                                    const core::Text &text);
    void slot_edited_easing_between(const EasingBetweenTwoFramesData &data);

    // Document
public:
    void emit_document_modified() override;
    void emit_updated_item(const TreeItem *item, std::vector<int> roles) override;
    void emit_property_added(PropertyGroup*, model::Property*, int) override;
    void emit_property_removed(PropertyGroup*, model::Property*, int) override;
    void emit_about_to_insert_layer(Layer*, int) override;
    void emit_inserted_layer(Layer*, int) override;
    void emit_about_to_remove_layer(Layer*, int) override;
    void emit_removed_layer(Layer*, int) override;
    void emit_composition_settings_updated() override;
    void emit_about_to_add_shape_item(model::ShapeLayer *shape_layer,
                                      model::Group *group,
                                      model::ShapeItem *shape,
                                      int shape_index) override;
    void emit_shape_item_added(model::ShapeLayer *shape_layer,
                               model::Group *group,
                               model::ShapeItem *shape,
                               int shape_index) override;
    int emit_about_to_remove_shape(model::ShapeLayer *shape_layer,
                                   model::Group *group,
                                   model::ShapeItem *shape,
                                   int shape_index) override;
    void emit_shape_removed(int shape_index) override;
    bool emit_about_to_move_layer(int from, int to) override;
    void emit_moved_layers(int from, int to) override;
    bool emit_about_to_move_shape(model::ShapeLayer *shape_layer,
                                  model::Object *object,
                                  int source_index,
                                  int destination_index) override;
    void emit_shape_moved(model::ShapeLayer *shape_layer,
                          model::Object *object,
                          int source_index,
                          int destination_index) override;
    void emit_text_layer_settings_changed(model::TextLayer *text_layer,
                                          const core::Text &text) override;

signals:
    void modified_changed(CompositionDocument *);
    void new_layer_added(Layer *layer, int index);
    void layer_moved(int from, int to);

    void shape_added(ShapeLayer *shape_layer, Group *group, ShapeItem *shape, int index);
    void about_to_remove_layer(Layer *layer);
    void about_to_remove_shape(ShapeLayer *layer, Group *group, ShapeItem *shape);
    void shapes_moved(ShapeLayer *layer, Object *group, int from, int to);
    void name_changed(QString);
    void id_changed(QUuid);
    void nodes_updated();
    void saved_path_changed(QString);

    void show_settings_for_text_layer(QVariant);
    void show_menu_for_layer(model::Layer *);
    void show_menu_for_property(model::Property *);
    void show_menu_for_shape_item(model::ShapeItem *);
    void composition_settings_updated();
    void layers_index_info_changed();
    void layer_index_changed(Layer *layer, int matte_layer_index, bool is_matte = true);
    void show_matte_details_changed(bool);

    // Text Layer
    void text_layer_settings_changed(model::TextLayer *text_layer,
                                     const core::Text &text);

    void apply_composition_script();

    void property_added(model::PropertyGroup *group, model::Property *prop, int index);
    void property_removed(model::PropertyGroup *group, model::Property *prop, int index);
    void applied_script() const;

private:
    bool set_item_data(TreeItem *item, int role, const QVariant &data);
    bool locked_role_value(const Property *property, const Object *object, const Layer *layer) const;
    bool layer_blink_role_value(const Property *property,
                                const Object *object,
                                const Layer *layer) const;
    QVariant drag_status_value(const Property *property,
                               const Object *object,
                               const Layer *layer) const;

    QVariant item_data(const TreeItem *property,
                       CompositionItemModel::Section /*section*/,
                       int role) const;

    bool blink_if_locked(TreeItem *property);
    void reset_uuid();
    void update_layer_index_info();

private:
    int m_new_layer_index = 1;
    QVariantList m_layers_index_info;
    std::map<int, QString> m_layers_index_to_name;
    std::string m_playground_script;
    TagModel *m_tag_model = nullptr;
    bool m_is_applying_script = false;
    friend class alive::DocumentUndoManager;
};
} // namespace alive::model
// namespace alive::model
#endif // DOCUMENTMODEL_H
