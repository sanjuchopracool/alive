#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <core/alive_types/class_helpers.h>
#include <core/alive_types/common_types.h>
#include <editor/undo_support/undostack.h>
#include <editor/settings/composition_setting.h>

namespace alive {
class ITagModel;
class DocumentUndoManager;

namespace core {
class Text;
}

namespace model {
class Composition;
class Object;
class PropertyGroup;
class Property;
class Layer;
class ShapeLayer;
class Group;
class ShapeItem;
class TextLayer;
} // namespace model

class TreeItem;
class Command;

class Document
{
    ONLY_CLONABLE(Document)
public:
    Document(model::Composition *comp);
    ~Document();

    void init();

    const std::string &saved_path() const { return m_saved_path; }
    void set_saved_path(const std::string &path);
    bool is_modified() const { return m_is_modified; }
    void set_modified(bool status);

    DocumentUndoManager *undo_manager();
    model::Composition *composition();
    const model::Composition *composition() const;
    virtual ITagModel* tag_model() = 0;

    CompositionSetting composition_setting() const;
    static model::Layer *layer_for_item(const TreeItem *item);

private:
    // layer related
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

public:
    // TODO signals
    virtual void emit_document_modified() = 0;
    virtual void emit_updated_item(const TreeItem *item, std::vector<int> roles) = 0;
    virtual void emit_property_added(model::PropertyGroup*, model::Property*, int) = 0;
    virtual void emit_property_removed(model::PropertyGroup*, model::Property*, int) = 0;
    virtual void emit_about_to_insert_layer(model::Layer*, int) = 0;
    virtual void emit_inserted_layer(model::Layer*, int) = 0;
    virtual void emit_about_to_remove_layer(model::Layer*, int) = 0;
    virtual void emit_removed_layer(model::Layer*, int) = 0;
    virtual void emit_composition_settings_updated() = 0;
    virtual void emit_about_to_add_shape_item(model::ShapeLayer *shape_layer,
                                              model::Group *group,
                                              model::ShapeItem *shape,
                                              int shape_index)
        = 0;
    virtual void emit_shape_item_added(model::ShapeLayer *shape_layer,
                                       model::Group *group,
                                       model::ShapeItem *shape,
                                       int shape_index)
        = 0;
    virtual int emit_about_to_remove_shape(model::ShapeLayer *shape_layer,
                                           model::Group *group,
                                           model::ShapeItem *shape,
                                           int shape_index)
        = 0;
    virtual void emit_shape_removed(int shape_index) = 0;
    virtual bool emit_about_to_move_layer(int from, int to) = 0;
    virtual void emit_moved_layers(int from, int to) = 0;
    virtual bool emit_about_to_move_shape(model::ShapeLayer *shape_layer,
                                          model::Object *object,
                                          int source_index,
                                          int destination_index)
        = 0;
    virtual void emit_shape_moved(model::ShapeLayer *shape_layer,
                                  model::Object *object,
                                  int source_index,
                                  int destination_index)
        = 0;
    virtual void emit_text_layer_settings_changed(model::TextLayer *text_layer,
                                                  const core::Text &text) = 0;
private:
    void push_command(alive_ptr<Command> &cmd);

protected:
    UndoStack m_document_undo_stack;
    alive_ptr<model::Composition> m_comp;
    alive_ptr<DocumentUndoManager> m_undo_manager;
    KeyFrameTime m_current_frame = 0;


private:
    std::string m_saved_path;
    bool m_is_modified = false;

    friend class DocumentUndoManager;
};

} // namespace alive

#endif // DOCUMENT_H
