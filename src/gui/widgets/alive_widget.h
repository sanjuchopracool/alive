#ifndef ALIVE_WIDGET_H
#define ALIVE_WIDGET_H

#include <QWidget>

namespace alive {
class WidgetInterface
{
public:
    struct UpdateContext
    {};
    virtual ~WidgetInterface() {}
    virtual void update(const UpdateContext &context) {}
};

} // namespace Alive
#endif // ALIVE_WIDGET_H
