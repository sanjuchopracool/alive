#ifndef GUI_INITIALIZER_H
#define GUI_INITIALIZER_H

#include <core/initializer.h>

namespace alive {
class Initializer
{
public:
    Initializer(const alive::core::Initializer::Params &params, bool qml = false);
    ~Initializer();

private:
    alive::core::Initializer m_core_initializer;
    bool m_qml = false;
};

} // namespace alive

#endif // INITIALIZER_H
