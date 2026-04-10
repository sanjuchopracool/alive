#include "gui_settings.h"
#include <core/asset_management/font_manager.h>
#include <gui/utility/gui_utility.h>

namespace alive::model {

bool SourceTextSetting::operator!=(const SourceTextSetting &other) const
{
    return (m_text != other.m_text);
}

SourceTextSetting from_text_document_property(const core::Text &text_properties)
{
    SourceTextSetting setting;
    setting.m_text = QString::fromStdString(text_properties.m_text);
    return setting;
}

void update_text_document_property(core::Text &prop,
                                   const SourceTextSetting &setting)
{
    prop.m_text = setting.m_text.toStdString();
}

bool StrokeSetting::operator!=(const StrokeSetting &other) const
{
    return (m_enable != other.m_enable) || (m_width != other.m_width) || (m_color != other.m_color)
           || (m_cap_style != other.m_cap_style) || (m_join_style != other.m_join_style);
}

bool FillSetting::operator!=(const FillSetting &other) const
{
    return (m_enable != other.m_enable) || (m_color != other.m_color);
}

bool FontSetting::operator!=(const FontSetting &other) const
{
    return (m_family != other.m_family) || (m_style != other.m_style) || (m_size != other.m_size);
}

FontSetting font_setting_from_text_document(const core::Text &prop)
{
    FontSetting setting;
    setting.m_family = prop.m_font.family;
    setting.m_style = prop.m_font.style;
    setting.m_size = prop.m_font.size;
    return setting;
}

void update_text_fill_setting(core::Text &prop, const FillSetting &setting)
{
    prop.m_fill = setting.m_enable;
    prop.m_fill_color = gui::qcolor_to_vec3d(setting.m_color);
}

FillSetting fill_setting_from_text_document(const core::Text &prop)
{
    FillSetting setting;
    setting.m_enable = prop.m_fill;
    setting.m_color = gui::qcolor_from_vec3d(prop.m_fill_color);
    return setting;
}

void update_text_stroke_setting(core::Text &prop, const StrokeSetting &setting)
{
    prop.m_stroke = setting.m_enable;
    prop.m_stroke_width = setting.m_width;
    prop.m_stroke_color = gui::qcolor_to_vec3d(setting.m_color);
    prop.m_cap_style = setting.m_cap_style;
    prop.m_join_style = setting.m_join_style;
}

StrokeSetting stroke_setting_from_text_document(const core::Text &text_properties)
{
    StrokeSetting setting;
    setting.m_enable = text_properties.m_stroke;
    setting.m_color = gui::qcolor_from_vec3d(text_properties.m_stroke_color);
    setting.m_width = text_properties.m_stroke_width;
    setting.m_cap_style = text_properties.m_cap_style;
    setting.m_join_style = text_properties.m_join_style;
    return setting;
}

void update_text_font_setting(core::Text &prop, const FontSetting &setting)
{
    prop.m_font.family = setting.m_family;
    prop.m_font.size = setting.m_size;
    prop.m_font.style = setting.m_style;
}

} // namespace alive::model
