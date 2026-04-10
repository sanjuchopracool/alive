include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library(editor
    editor/undo_support/command.h
    editor/undo_support/undostack.h editor/undo_support/undostack.cpp
    editor/undo_support/functor_commands.h editor/undo_support/functor_commands.cpp
    editor/document/document.h editor/document/document.cpp
    editor/undo_support/command_enums.h
    editor/undo_support/object_commands.cpp editor/undo_support/object_commands.h
    editor/tagging/ITagModel.h
    editor/undo_support/document_undo_manager.cpp editor/undo_support/document_undo_manager.h
    editor/undo_support/layer_command.h editor/undo_support/layer_command.cpp
    editor/interfaces/timeline.h
    editor/undo_support/timeline_commands.cpp editor/undo_support/timeline_commands.h
    editor/undo_support/document_commands.cpp editor/undo_support/document_commands.h
    editor/settings/composition_setting.h
    editor/undo_support/shape_item_commands.cpp editor/undo_support/shape_item_commands.h
    editor/undo_support/layer_commands.cpp editor/undo_support/layer_commands.h
    editor/undo_support/keyframe_commands.cpp editor/undo_support/keyframe_commands.h
    editor/undo_support/property_commands.cpp editor/undo_support/property_commands.h
    editor/settings/file_based_layer_setting.h editor/settings/layer_setting.cpp editor/settings/layer_setting.h editor/settings/solid_layer_setting.h editor/settings/text_layer_setting.h )

target_link_libraries(editor PUBLIC core)
target_include_directories(editor PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
