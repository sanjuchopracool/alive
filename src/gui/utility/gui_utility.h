#ifndef GUI_UTILITY_H
#define GUI_UTILITY_H

#include <core/alive_types/common_types.h>
#include <gui/model/layers/gui_settings.h>
#include <gui/model/property/gui_property.h>
#include <QFontDatabase>
#include <QObject>

QT_BEGIN_NAMESPACE
class QQmlEngine;
class QJSEngine;
QT_END_NAMESPACE

namespace alive::model {
class Property;
class Object;
class Layer;
class CompositionDocument;
} // namespace alive::model
namespace alive::gui {

inline QColor qcolor_from_vec3d(const Vec3D &value)
{
    return QColor(value.x() * 255, value.y() * 255, value.z() * 255);
}

inline Vec3D qcolor_to_vec3d(const QColor &color)
{
    return Vec3D(color.red() / 255.0, color.green() / 255.0, color.blue() / 255.0);
}

class Utility : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(Utility)

    Q_PROPERTY(const QStringList &blend_modes READ blend_modes CONSTANT)
    Q_PROPERTY(const QStringList &matte_modes MEMBER m_matte_modes CONSTANT)
    Q_PROPERTY(model::SourceTextSetting text_setting READ text_setting WRITE set_text_setting NOTIFY
                   text_setting_changed)
    Q_PROPERTY(model::StrokeSetting stroke_setting READ stroke_setting WRITE set_stroke_setting
                   NOTIFY stroke_setting_changed)

    Q_PROPERTY(model::FillSetting fillSetting READ fill_setting WRITE set_fill_setting NOTIFY
                   fill_setting_changed)

    Q_PROPERTY(model::FontSetting font_setting READ font_setting WRITE set_font_setting NOTIFY
                   font_setting_changed)
public:
    Q_INVOKABLE bool validate_object_name(const QString &object_name);

private:
    explicit Utility(QObject *parent = nullptr);

public:
    static QObject *qml_instance(QQmlEngine *engine, QJSEngine *script_engine)
    {
        Q_UNUSED(engine);
        Q_UNUSED(script_engine);
        return instance();
    }

    static void init();
    static void deinit(bool qml = false);
    static Utility *instance();

    QVariant edit_value(const model::Property *property,
                        const model::Object *object,
                        const model::Layer *layer) const;
    bool set_edit_value(model::CompositionDocument *document,
                        model::Property *property,
                        model::Object *object,
                        model::Layer *layer,
                        KeyFrameTime time,
                        QVariant var);
    const QStringList &blend_modes() const;
    const QStringList &matte_modes() const;

    model::SourceTextSetting text_setting() { return m_text_setting; }
    void set_text_setting(model::SourceTextSetting new_setting);

    model::StrokeSetting stroke_setting() { return m_stroke_setting; }
    void set_stroke_setting(model::StrokeSetting &new_setting);

    model::FillSetting fill_setting() const;
    void set_fill_setting(const model::FillSetting &new_setting);

    model::FontSetting font_setting() const;
    void set_font_setting(const model::FontSetting &new_setting);
    void update_font_setting(const model::FontSetting &new_setting);

    const QString &matte_mode(int index) const;
signals:
    void text_setting_changed();
    void stroke_setting_changed();

    void fill_setting_changed();

    void font_setting_changed();

private:
    QStringList m_blend_modes;
    QStringList m_matte_modes;
    model::SourceTextSetting m_text_setting;
    model::StrokeSetting m_stroke_setting;
    model::FillSetting m_fill_setting;
    model::FontSetting m_font_setting;
};
} // namespace alive::gui

#endif // GUI_UTILITY_H
