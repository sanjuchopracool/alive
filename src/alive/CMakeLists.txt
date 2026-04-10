set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Alive
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        application.h application.cpp
        action_ids.h


    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET Alive APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(Alive SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(Alive
            ${PROJECT_SOURCES}
        )
    endif()
endif()

qt_add_resources(Alive "fonts"
    PREFIX ""
    FILES fonts/Roboto/Roboto-ThinItalic.ttf fonts/Roboto/Roboto-Thin.ttf fonts/Roboto/Roboto-Regular.ttf fonts/Roboto/Roboto-MediumItalic.ttf fonts/Roboto/Roboto-Medium.ttf fonts/Roboto/Roboto-LightItalic.ttf fonts/Roboto/Roboto-Light.ttf fonts/Roboto/Roboto-Italic.ttf fonts/Roboto/Roboto-BoldItalic.ttf fonts/Roboto/Roboto-Bold.ttf fonts/Roboto/Roboto-BlackItalic.ttf fonts/Roboto/Roboto-Black.ttf
)

target_link_libraries(Alive PRIVATE Qt${QT_VERSION_MAJOR}::Widgets gui)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.Alive)
endif()
set_target_properties(Alive PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS Alive
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Alive)
endif()
