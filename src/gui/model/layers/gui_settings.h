#ifndef GUI_SETTINGS_H
#define GUI_SETTINGS_H

#include <QColor>
#include <QObject>
#include <QString>

#include <core/model/text_primitives/text.h>

namespace alive::model {
struct FillSetting
{
    Q_GADGET
    Q_PROPERTY(bool enable MEMBER m_enable)
    Q_PROPERTY(QColor color MEMBER m_color)

public:
    bool operator!=(const FillSetting &other) const;
    auto operator<=>(const FillSetting &other) const = delete;

    bool m_enable = true;
    QColor m_color = Qt::lightGray;
};
///////////////////////////////////////////////////////////////////////////////
struct FontSetting
{
    Q_GADGET
    Q_PROPERTY(qreal size MEMBER m_size)
    Q_PROPERTY(int family MEMBER m_family)
    Q_PROPERTY(int style MEMBER m_style)

public:
    bool operator!=(const FontSetting &other) const;
    qreal m_size = 20;
    int m_family;
    int m_style;
};
///////////////////////////////////////////////////////////////////////////////

struct StrokeSetting
{
public:
    bool operator!=(const StrokeSetting &other) const;
    auto operator<=>(const StrokeSetting &other) const = delete;

    bool m_enable = false;
    qreal m_width = 1;
    QColor m_color = Qt::black;
    CapStyle m_cap_style = CapStyle::e_Butt;
    JoinStyle m_join_style = JoinStyle::e_Miter;
};

struct SourceTextSetting
{
    Q_GADGET
    Q_PROPERTY(QString text MEMBER m_text)
public:
    bool operator!=(const SourceTextSetting &other) const;
    auto operator<=>(const SourceTextSetting &other) const = delete;

    QString m_text;
};
SourceTextSetting from_text_document_property(const core::Text &prop);
void update_text_document_property(core::Text &prop,
                                   const SourceTextSetting &setting);
void update_text_font_setting(core::Text &prop, const FontSetting &setting);
FontSetting font_setting_from_text_document(const core::Text &prop);

void update_text_fill_setting(core::Text &prop, const FillSetting &setting);
FillSetting fill_setting_from_text_document(const core::Text &prop);

void update_text_stroke_setting(core::Text &prop, const StrokeSetting &setting);
StrokeSetting stroke_setting_from_text_document(const core::Text &prop);

} // namespace alive::model

#endif // GUI_SETTINGS_H
