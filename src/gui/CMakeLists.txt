find_package(Boost)
include_directories(${Boost_INCLUDE_DIRS})

set (ALIVE_GUI_CORE_FILES_ALL
    model/composition_item_model.h
    model/composition_item_model.cpp
    document/composition_document.h
    document/composition_document.cpp
)

set (ALIVE_GUI_HEADER_FILES_ALL
    theme/canvas_color_setting.h
    settings/user_settings.h
    settings/config_constants.h
    theme/timeline_style.h

    theme/layer_color_model.h
    composition/timeline.h
)

set (ALIVE_GUI_SRC_FILES_ALL
    settings/user_settings.cpp
    theme/timeline_style.cpp

    theme/layer_color_model.cpp
    composition/timeline.cpp
    model/property/property_editor_callbacks.h
    model/property/property_editor_callbacks.cpp


    color_utility.h
    settings/timeline_settings.h
    settings/timeline_settings.cpp
)

set (ALIVE_GUI_DIRECTORY_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/")
list(TRANSFORM ALIVE_GUI_HEADER_FILES_ALL PREPEND ${ALIVE_GUI_DIRECTORY_PREFIX})
list(TRANSFORM ALIVE_GUI_CORE_FILES_ALL PREPEND ${ALIVE_GUI_DIRECTORY_PREFIX})

add_library(gui
    ${ALIVE_GUI_HEADER_FILES_ALL}
    ${ALIVE_GUI_SRC_FILES_ALL}
    ${ALIVE_GUI_CORE_FILES_ALL}
    framework/theme/color_palette.h
    framework/theme/color_palette.cpp

    framework/theme/alive_style.h
    framework/theme/alive_style.cpp
    gui_utility.h

    quick/timeline_ruler.h quick/timeline_ruler.cpp
    quick/range_selector_helper.h quick/range_selector_helper.cpp
    quick/range_selector_handle.h quick/range_selector_handle.cpp
    cursor_manager.h cursor_manager.cpp
    model/undostack_model.h model/undostack_model.cpp
    project/project_manager.h project/project_manager.cpp
    quick/keyframe_item.h quick/keyframe_item.cpp
    utility/gui_utility.h utility/gui_utility.cpp
    model/property/gui_property.h
    model/layers/shape_layer_modifier.h model/layers/shape_layer_modifier.cpp

    font_model.h font_model.cpp
    model/layers/gui_settings.h
    model/layers/gui_settings.cpp
    timeline_model_helpers.h timeline_model_helpers.cpp
    document/composition_document_model.h document/composition_document_model.cpp

    document/factories/editor_layer_factory.h document/factories/editor_layer_factory.cpp
    document/composition_document_proxy_model.h document/composition_document_proxy_model.cpp
    utility/font_proxy_filter_model.h utility/font_proxy_filter_model.cpp
    settings/user_ui_setting.cpp settings/user_ui_setting.h
    gui_initializer.h gui_initializer.cpp
    widgets/timeline_range_selector.h
    widgets/timeline_range_selector.cpp
    widgets/timeline_property_editor.cpp
    widgets/timeline_property_editor.h
    widgets/timeline_ruler.cpp
    widgets/timeline_ruler.h
    widgets/alive_widget.h
    widgets/utility.h widgets/utility.cpp
    widgets/timeline_range_selector_item.h widgets/timeline_range_selector_item.cpp
    widgets/canvas_widget.h widgets/canvas_widget.cpp
    widgets/main_window.h widgets/main_window.cpp
    widgets/dock_widget.h widgets/dock_widget.cpp
    widgets/tab_widget.h widgets/tab_widget.cpp
    widgets/timeline_tab_widget.h widgets/timeline_tab_widget.cpp
    widgets/canvas_tab_widget.h widgets/canvas_tab_widget.cpp
    widgets/central_widget.h widgets/central_widget.cpp
    widgets/timeline_widget.h widgets/timeline_widget.cpp
    widgets/timeline_playhead.h widgets/timeline_playhead.cpp
    widgets/menubar.h widgets/menubar.cpp

    managers/dock_action_manager.h managers/dock_action_manager.cpp
    widgets/timeline_treeview.h widgets/timeline_treeview.cpp
    delegates/timeline_column_delegate.h delegates/timeline_column_delegate.cpp
    delegates/feature_column_delegate.h delegates/feature_column_delegate.cpp
    delegates/tree_column_delegate.h delegates/tree_column_delegate.cpp
    delegates/layer_column_delegate.h delegates/layer_column_delegate.cpp
    widgets/timeline_headerview.h widgets/timeline_headerview.cpp
    delegates/edit_column_delegate.h delegates/edit_column_delegate.cpp
    delegates/delegate_utility.h delegates/delegate_utility.cpp
    utility/keyframe_gui_utility.h utility/keyframe_gui_utility.cpp
    icon_manager.cpp
    icon_manager.h icon_manager.cpp
    utility/keyframe_selection_manager.h utility/keyframe_selection_manager.cpp
    dialogs/composition_setting_dialog.h dialogs/composition_setting_dialog.cpp dialogs/composition_setting_dialog.ui
    widgets/checkable_icon.cpp widgets/checkable_icon.h
    widgets/label_editor.cpp widgets/label_editor.h
    widgets/line_editor.cpp widgets/line_editor.h
    widgets/linked_number_editor.cpp widgets/linked_number_editor.h
    dialogs/solid_layer_setting_dialog.h dialogs/solid_layer_setting_dialog.cpp dialogs/solid_layer_setting_dialog.ui
    widgets/number_editor.cpp widgets/number_editor.h
    widgets/color_selection_label.h widgets/color_selection_label.cpp
    widgets/color_line_editor.h widgets/color_line_editor.cpp
    widgets/edit_column_header.h widgets/edit_column_header.cpp
    framework/managers/action_manager.h framework/managers/action_manager.cpp
    action_contexts.h
    framework/canvas/canvas_interface.h framework/canvas/canvas_interface.cpp
    framework/widgets/bezier_curve_editor.h framework/widgets/bezier_curve_editor.cpp
    dialogs/file_based_layer_setting_dialog.h dialogs/file_based_layer_setting_dialog.cpp dialogs/file_based_layer_setting_dialog.ui
    framework/widgets/timeline_expression_editor.h framework/widgets/timeline_expression_editor.cpp
    framework/theme/messagebox.h framework/theme/messagebox.cpp
    widgets/textproperty_widget.h widgets/textproperty_widget.cpp widgets/textproperty_widget.ui
    widgets/exclusive_toolbutton_group.h widgets/exclusive_toolbutton_group.cpp

    framework/widgets/toolbar.h framework/widgets/toolbar.cpp
    framework/widgets/toolbar_property_widget.h framework/widgets/toolbar_property_widget.cpp
    project/global_selection_manager.h project/global_selection_manager.cpp
    widgets/timeline_header_widget.h widgets/timeline_header_widget.cpp
    widgets/property_group_widget.h widgets/property_group_widget.cpp
    framework/widgets/property_editor/property_widget_base.h framework/widgets/property_editor/property_widget_base.cpp
    framework/widgets/property_editor/property_widget_factory.h framework/widgets/property_editor/property_widget_factory.cpp
    framework/widgets/script_editor_dialog.h framework/widgets/script_editor_dialog.cpp framework/widgets/script_editor_dialog.ui
    framework/widgets/new_property_dialog.h framework/widgets/new_property_dialog.cpp framework/widgets/new_property_dialog.ui
    tagging/tag_model.h tagging/tag_model.cpp
    widgets/tag_managememt_widget.h widgets/tag_managememt_widget.cpp
    widgets/font_setting_widget.h widgets/font_setting_widget.cpp widgets/font_setting_widget.ui
)

# Resources:
#set(icons_resource_file
#    "icons/dark/actions/16@2x/link.png"
#    "icons/dark/actions/16/link.png"
#)

#qt_add_resources(gui "icons"
#    PREFIX
#        "/"
#    FILES
#        ${icons_resource_file}
#)

#qt_add_resources(alive_test_app "stylesheet"
#    PREFIX
#    "/"
#    FILES
#    "styles/dark_style.css"
#)

target_link_libraries(gui PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Quick PUBLIC core editor)

cmake_path(GET CMAKE_CURRENT_SOURCE_DIR PARENT_PATH ALIVE_GUI_SOURCE_PARENT_DIR)
target_include_directories(gui PUBLIC ${ALIVE_GUI_SOURCE_PARENT_DIR})

#add_subdirectory(tests)
