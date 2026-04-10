#ifndef LAYER_COMMAND_H
#define LAYER_COMMAND_H

#include <editor/undo_support/command.h>

namespace alive {

namespace model {
class Layer;
}

class LayerCommand : public Command
{
public:
    LayerCommand(model::Layer *layer, CommandType type,
                 const std::string &name = {})
        : Command(type, name),
        m_layer(layer)
    {}
    const model::Layer *layer() const { return m_layer; }

protected:
    model::Layer *m_layer;
};

class AddRemoveLayerCommand : public LayerCommand
{
public:
    enum Type { e_Add, e_Remove };

    AddRemoveLayerCommand(const std::vector<model::Layer *> &layers,
                          Type type = e_Add,
                          int index = -1);
    ~AddRemoveLayerCommand();
    void redo() override;
    void undo() override;

private:
    void add_layers();
    void remove_layers();

private:
    std::vector<model::Layer *> m_layers;
    std::vector<int> m_indices;
    Type m_type;
    int m_index = -1;
    bool m_deleted = false;
};

} // namespace alive

#endif // LAYER_COMMAND_H
