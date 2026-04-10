#include "gui_initializer.h"
#include <gui/framework/managers/action_manager.h>
#include <gui/framework/theme/color_palette.h>
#include <gui/icon_manager.h>
#include <gui/managers/dock_action_manager.h>
#include <gui/project/project_manager.h>
#include <gui/settings/user_ui_setting.h>
#include <gui/theme/layer_color_model.h>
#include <gui/theme/timeline_style.h>
#include <gui/utility/gui_utility.h>
#include <gui/utility/keyframe_gui_utility.h>
#include <gui/widgets/timeline_treeview.h>
namespace alive {

namespace {
static int init_counter = 0;
}

void init()
{
    alive::UserSettings::init();
    alive::theme::ColorPalette::init();
    alive::gui::Utility::init();
    alive::ActionManager::init();
    alive::DockActionManager::init();
    alive::project::ProjectManager::init();
    alive::theme::TimelineSize::init();
    alive::gui::LayerColorModel::init();
    alive::keyFrameGuiUtility::generate_keyframe_path();
    alive::IconManager::init();
    alive::init_timeline_globals();
}

void de_init(bool qml)
{
    alive::IconManager::deinit(qml);
    alive::gui::LayerColorModel::deinit(qml);
    alive::project::ProjectManager::deinit(qml);
    alive::theme::TimelineSize::deinit(qml);
    alive::DockActionManager::deinit();
    alive::ActionManager::deinit();
    alive::gui::Utility::deinit(qml);
    alive::theme::ColorPalette::deinit(qml);
    alive::UserSettings::deinit();
}

Initializer::Initializer(const alive::core::Initializer::Params &params, bool qml)
    : m_core_initializer(params)
    , m_qml(qml)
{
    if (init_counter++ == 0) {
        init();
    }
}

Initializer::~Initializer()
{
    if (--init_counter == 0) {
        de_init(m_qml);
    }
}
} // namespace alive
