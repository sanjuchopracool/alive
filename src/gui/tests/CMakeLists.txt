message(STATUS "core_tests enabled")

qt_add_executable(core_tests
model/property/property_descriptor_tests.h
model/property/property_descriptor_tests.cpp


main.cpp
model/property/numeric_property_tests.h
model/property/numeric_property_tests.cpp
model/composition_tests.h
model/composition_tests.cpp
)

set_target_properties(core_tests PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

qt_add_resources(core_tests "FILES"
    PREFIX
    "/"
    FILES
    "SOLID_LAYER.json"
)
target_link_libraries(core_tests PRIVATE
    core
    gui
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Test
)
