#include "icon_manager.h"

namespace alive {

namespace {
static IconManager *the_instance = nullptr;
}

struct IconManagerPrivateData
{
    std::vector<QIcon> icons;
};

IconManager::IconManager()
    : m_d(std::make_unique<IconManagerPrivateData>())
{
    std::vector<QIcon> &icons = m_d->icons;
    icons.resize(e_Last);
    icons[e_AnimationDisabled] = QIcon::fromTheme("animation_disabled");
    icons[e_AnimationEnabled] = QIcon::fromTheme("animation_enabled");
    icons[e_BoxChecked] = QIcon::fromTheme("box_checked");
    icons[e_BoxUnChecked] = QIcon::fromTheme("box_unchecked");
    icons[e_Visible] = QIcon::fromTheme("eye");
    icons[e_Lock] = QIcon::fromTheme("lock");
    icons[e_AtScriptedKeyFrame] = QIcon::fromTheme("at_keyframe_scripted");
    icons[e_AtKeyFrame] = QIcon::fromTheme("at_keyframe");
    icons[e_NotAtKeyFrame] = QIcon::fromTheme("not_at_keyframe");
    icons[e_KeyframeAtLeft] = QIcon::fromTheme("left_keyframe");
    icons[e_ScriptedKeyframeAtLeft] = QIcon::fromTheme("left_keyframe_scripted");
    icons[e_KeyframeNotAtLeft] = QIcon::fromTheme("left_keyframe_disabled");
    icons[e_KeyframeAtRight] = QIcon::fromTheme("right_keyframe");
    icons[e_ScriptedKeyframeAtRight] = QIcon::fromTheme("right_keyframe_scripted");
    icons[e_KeyframeNotAtRight] = QIcon::fromTheme("right_keyframe_disabled");
    icons[e_ComboDownArrow] = QIcon::fromTheme("expand");
    icons[e_Grid] = QIcon::fromTheme("map-flat");
    icons[e_GroupAdd] = QIcon::fromTheme("group_add");
    icons[e_Link] = QIcon::fromTheme("link");
    icons[e_MsgBoxInformation] = QIcon::fromTheme("dialog-information");
    icons[e_MsgBoxWarning] = QIcon::fromTheme("dialog-warning");
    icons[e_MsgBoxError] = QIcon::fromTheme("dialog-error");
    icons[e_MsgBoxCritical] = QIcon::fromTheme("dialog-critical");
}

void IconManager::init()
{
    if (!the_instance) {
        the_instance = new IconManager();
    }
}

void IconManager::deinit(bool qml)
{
    if (!qml && the_instance) {
        delete the_instance;
        the_instance = nullptr;
    }
}

IconManager *IconManager::instance()
{
    return the_instance;
}

const QIcon &IconManager::get_icon(Icon index) const
{
    Q_ASSERT(index < e_Last);
    return m_d->icons[index];
}
} // namespace alive
