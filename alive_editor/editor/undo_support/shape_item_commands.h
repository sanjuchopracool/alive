#ifndef SHAPEITEMCOMMANDS_H
#define SHAPEITEMCOMMANDS_H

#include <core/alive_types/common_types.h>
#include <editor/undo_support/command.h>

namespace alive::model {
class ShapeItem;
class PathShapeItem;

} // namespace alive::model
namespace alive {
class ShapeItemCommand : public Command
{
public:
    ShapeItemCommand(model::ShapeItem *shape, CommandType type)
        : Command(type, "")
        , m_shape(shape)
    {}

    const model::ShapeItem *shape_tem() const { return m_shape; }
    void update_document();

protected:
    model::ShapeItem *m_shape;
};

class SetPathShapeDirectionCommand : public ShapeItemCommand
{
public:
    SetPathShapeDirectionCommand(model::PathShapeItem *shape, PathDirection new_direction);

    const model::PathShapeItem *path_shape_tem() const { return m_path_shape; }
    void redo() override;
    void undo() override;

protected:
    model::PathShapeItem *m_path_shape;
    PathDirection m_new_direction;
    PathDirection m_old_direction;
};

} // namespace alive

#endif // SHAPEITEMCOMMANDS_H
