#ifndef ICON_MANAGER_H
#define ICON_MANAGER_H

#include <memory>
#include <QIcon>

namespace alive {
struct IconManagerPrivateData;
class IconManager
{
public:
    enum Icon {
        e_AnimationDisabled,
        e_AnimationEnabled,
        e_BoxChecked,
        e_BoxUnChecked,
        e_Visible,
        e_Lock,
        e_AtKeyFrame,
        e_AtScriptedKeyFrame,
        e_NotAtKeyFrame,
        e_KeyframeAtLeft,
        e_ScriptedKeyframeAtLeft,
        e_KeyframeNotAtLeft,
        e_KeyframeAtRight,
        e_ScriptedKeyframeAtRight,
        e_KeyframeNotAtRight,
        e_ComboDownArrow,
        e_Grid,
        e_GroupAdd,
        e_Link,
        e_MsgBoxInformation,
        e_MsgBoxWarning,
        e_MsgBoxError,
        e_MsgBoxCritical,
        e_Last
    };

    static void init();
    static void deinit(bool qml = false);
    static IconManager *instance();
    const QIcon &get_icon(Icon index) const;

private:
    IconManager();

private:
    std::unique_ptr<IconManagerPrivateData> m_d;
};
} // namespace alive

#endif // ICON_MANAGER_H
