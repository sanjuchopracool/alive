#ifndef DOCUMENT_UNDO_MANAGER_H
#define DOCUMENT_UNDO_MANAGER_H

#include <editor/undo_support/undostack.h>
#include <editor/tagging/ITagModel.h>
#include <editor/settings/layer_setting.h>

namespace alive {

class Tag;
class Taggable;
class ITagModel;
class Document;
class ITimeline;
class CompositionSetting;
class TreeItem;

namespace core
{
class Text;
}

namespace model {
class Composition;
enum class PropertyType;
class Property;
class PropertyGroup;
class Object;
class Layer;
class PathShapeItem;
class ShapeLayer;
class Group;
class ShapeItem;
class KeyFrame;
class TextLayer;
enum class MatteType;
enum class BlendMode;
class EasingBetweenTwoFramesData;
} // namespace model

class DocumentUndoManager
{
public:
    explicit DocumentUndoManager(Document *doc);

    /*********************************************************************************
    *                 Do/Undo Support
    *********************************************************************************/
    void redo();
    void undo();

    /*********************************************************************************
    *                 Tagging Support
    *********************************************************************************/
    void tag(Taggable *to, Tag *tag);
    void remove_tag(Taggable *from, Tag *tag);
    void create_tag(const std::string &name);
    void delete_with_context(const std::vector<ITagModel::TagDeleteContext> &selection);

    /*********************************************************************************
    *                 Property Group Related
    *********************************************************************************/

    void delete_property(model::PropertyGroup *group, model::Property *prop);
    void duplicate_property(model::PropertyGroup *group, model::Property *prop);
    void move_up_property(model::PropertyGroup *group, model::Property *prop);
    void add_property(model::PropertyGroup *group,
                      model::PropertyType type,
                      const std::string &name,
                      const std::string &id);

    /*********************************************************************************
    *                 Object Related
    *********************************************************************************/
    void set_object_visible(model::Object *object, bool visible);
    bool set_object_name(model::Object *obj, const std::string &name,std::vector<int> roles);

    /*********************************************************************************
    *                 Layer Related
    *********************************************************************************/
    void insert_new_layers(const std::vector<model::Layer *> &layers, int index = -1);
    void remove_layers(const std::vector<model::Layer *> &layers);
    void set_layer_in_out_points(model::Layer *layer,
                                 KeyFrameTime in_point,
                                 KeyFrameTime out_point,
                                 std::vector<int> roles);
    void set_layer_color_index(model::Layer *layer, int new_index);
    void set_layer_lock(model::Layer *layer, bool lock);
    void move_layers(int source_index, int destination_index);
    void set_layer_matte_type(model::MatteType change, model::MatteType old, model::Layer* layer,std::vector<int> roles);
    void set_layer_blend_mode(model::BlendMode change, model::BlendMode old, model::Layer* layer,std::vector<int> roles);
    void set_layer_matte_layer(int change, int old, model::Layer* layer,std::vector<int> roles);
    void set_layer_parent_layer(int change, int old, model::Layer* layer,std::vector<int> roles);
    void update_text_layer_settings(model::TextLayer *text_layer, const core::Text &text);
    void set_layer_setting(model::Layer* layer, const model::LayerSetting& setting ,std::vector<int> roles);

        /*********************************************************************************
    *                 WORK AREA RELATED
    *********************************************************************************/
        void set_work_area_start(ITimeline *timeline, int frame, bool merge = false);
    void set_work_area_end(ITimeline *timeline, int frame, bool merge = false);
    void set_work_area(ITimeline *timeline, int start, int end, bool merge = false);
    void set_visible_range_area(ITimeline *timeline, int start, int end, bool merge = false);

    /*********************************************************************************
    *                 Composition Setting
    *********************************************************************************/
    void update_composition_setting(const CompositionSetting &setting);
    void set_apply_script(const std::string& script);

    /*********************************************************************************
    *                 Updated Items Related
    *********************************************************************************/
    void updated_item(const TreeItem *item, std::vector<int> roles = {});

    /*********************************************************************************
    *                 Shape Items Related
    *********************************************************************************/
    bool set_path_direction(model::PathShapeItem *item, bool value, std::vector<int> roles);
    bool add_shape_item_to_shape_group(model::Object *object, int item_index);
    void remove_shape_item(model::ShapeItem *shape_item);
    void move_shape_item(model::ShapeLayer *shape_layer,
                         model::Object *object,
                         int source_index,
                         int destination_index);

    /*********************************************************************************
    *                 Property related
    *********************************************************************************/
    void set_expression_on_property(model::Property *property,
                                    const std::string &expression,
                                    KeyFrameTime time = -1);
    void disable_property_expression(model::Property *property, bool disable);
    void disable_keyframe_expression(model::KeyFrame* keyframe, bool disable);
    void move_keyframes(const std::set<model::KeyFrame *> &keyframes, KeyFrameTime t,std::vector<int> roles);
    void property_remove_keyframe_at(model::Property *property, KeyFrameTime t,std::vector<int> roles);
    void property_add_keyframe_at(model::Property *property, KeyFrameTime t,std::vector<int> roles);
    void property_remove_all_keyframes(model::Property *property, std::vector<int> roles);

    void update_easing_between_keyrames(const model::EasingBetweenTwoFramesData& data);
    bool set_linked(model::Property *property, bool value, std::vector<int> roles);
    template<typename T>
    bool change_dynamic_property_value(model::Property *property,
                                       KeyFrameTime time,
                                       const T &value,
                                       std::vector<int> roles);
    void update_property_change(model::Property *property,
                                KeyFrameTime time,
                                EditingField field,
                                int change,
                                std::vector<int> roles);
    bool update_property_change(model::Property *property,
                                KeyFrameTime frame,
                                EditingField field,
                                const std::string &new_value_str,
                                std::vector<int> roles);
private:
    int add_layer(model::Layer *layer, int index = 0);
    int remove_layer(model::Layer *layer);

    void update_composition_setting_impl(const CompositionSetting &setting);
    // shape related
    int add_shape(model::ShapeLayer *shape_layer,
                  model::Group *group,
                  model::ShapeItem *shape,
                  int shape_index = -1);
    int remove_shape(model::ShapeLayer *shape_layer, model::Group *group, model::ShapeItem *shape);
    bool move_shape(model::ShapeLayer *shape_layer,
                    model::Object *object,
                    int source_index,
                    int destination_index);
    bool move_layer(int source_index, int destination_index);

private:
    void push_cmd(alive_ptr<Command> &cmd);

private:
    Document *m_doc = nullptr;
    model::Composition *m_comp = nullptr;
    ITagModel *m_tag_model = nullptr;
    friend class AddRemoveLayerCommand;
    friend class CompositionSettingCommand;
    friend class AddRemoveShapeItemCommand;
    friend class MoveShapesCommand;
    friend class MoveLayersCommand;
};

} // namespace alive

#endif // DOCUMENT_UNDO_MANAGER_H
