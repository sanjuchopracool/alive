#ifndef LAYER_COMMANDS_H
#define LAYER_COMMANDS_H

#include <core/model/layers/layer.h>
#include <core/model/shape_items/shape_item.h>
#include <editor/undo_support/command.h>
#include <editor/undo_support/layer_command.h>
#include <editor/settings/layer_setting.h>

namespace alive::model {
class Layer;
}

namespace alive {

class DragLayerRangeCommand : public LayerCommand
{
public:
    enum Type { e_Start, e_End, e_Range };
    DragLayerRangeCommand(Type type,
                          model::Layer *layer,
                          FrameTimeType t,
                          const std::string &text = {});
    void redo() override;
    void undo() override;
    bool merge_with(const Command *other) override;

    FrameTimeType delta_time() const { return m_delta_frame; }
    Type type() const { return m_type; }

private:
    void apply_change(FrameTimeType delta);

private:
    Type m_type = e_Range;
    FrameTimeType m_delta_frame;
};

class SetLayerRangeCommand : public LayerCommand
{
public:
    SetLayerRangeCommand(model::Layer *layer, KeyFrameTime in, KeyFrameTime out);
    void redo() override;
    void undo() override;
    bool merge_with(const Command *other) override;

    KeyFrameTime in_point() const { return m_in; }
    KeyFrameTime out_point() const { return m_out; }

private:
    void update_text();

private:
    KeyFrameTime m_in = 0;
    KeyFrameTime m_out = 0;
    KeyFrameTime m_in_old = 0;
    KeyFrameTime m_out_old = 0;
};

class SetLayerBooleanCommand : public LayerCommand
{
public:
    enum Type { e_Visible, e_Lock };
    SetLayerBooleanCommand(Type type, bool flat, model::Layer *layer, const std::string &text = {});
    void redo() override;
    void undo() override;
    bool flag() const { return m_flag; }
    Type type() const { return m_type; }

private:
    void apply_change(bool flag);

private:
    Type m_type = e_Visible;
    bool m_flag = true;
};

class SetLayerIntCommand : public LayerCommand
{
public:
    enum Type { e_ColorIndex };
    SetLayerIntCommand(Type type, int value, model::Layer *layer, const std::string &text = {});
    void redo() override;
    void undo() override;
    bool merge_with(const Command *other) override;
    int value() const { return m_value; }
    Type type() const { return m_type; }

private:
    void update_text();
    void apply_change(int value);

private:
    Type m_type = e_ColorIndex;
    int m_value = 0;
    int m_old_value = 0;
};

class MoveLayersCommand : public Command
{
public:
    MoveLayersCommand(int from, int to);
    void redo() override;
    void undo() override;
    bool merge_with(const Command *other) override;

private:
    int m_from;
    int m_to;
};

class EditorLayerSettingCommand : public Command
{
public:
    EditorLayerSettingCommand(model::Layer *layer, const model::LayerSetting &setting);

    void redo() override;
    void undo() override;
    bool merge_with(const Command *other) override;
    void update_document();

private:
    model::Layer *m_layer = nullptr;
    model::LayerSetting m_setting;
    model::LayerSetting m_old_setting;
};

class AddRemoveShapeItemCommand : public LayerCommand
{
public:
    enum Type { e_Add, e_Remove };

    AddRemoveShapeItemCommand(model::ShapeLayer *layer,
                              model::Group *group,
                              model::ShapeItem *shape,
                              int index,
                              Type type = e_Add);
    void redo() override;
    void undo() override;

private:
    void add_shape();
    void remove_shape();

private:
    model::ShapeLayer *m_shape_layer = nullptr;
    model::Group *m_group = nullptr;
    model::ShapeItem *m_shape = nullptr;
    Type m_type;
    int m_index = -1;
};

class MoveShapesCommand : public Command
{
public:
    MoveShapesCommand(model::ShapeLayer *shape_layer, model::Object *object, int from, int to);
    void redo() override;
    void undo() override;
    bool merge_with(const Command *other) override { return false; }

private:
    model::ShapeLayer *m_shape_layer = nullptr;
    model::Object *m_object = nullptr;
    int m_from;
    int m_to;
};
} // namespace alive

#endif // LAYER_COMMANDS_H
