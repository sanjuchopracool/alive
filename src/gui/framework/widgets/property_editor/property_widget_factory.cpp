#include "property_widget_factory.h"
#include <core/font.h>
#include <core/model/property/property.h>
#include <core/model/property/property_type.h>
#include <core/model/property/static_property.h>
#include <gui/color_utility.h>
#include <gui/framework/widgets/property_editor/property_widget_base.h>
#include <gui/widgets/color_line_editor.h>
#include <gui/widgets/font_setting_widget.h>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QComboBox>

namespace alive::property {
QWidget *PropertyWidgetFactory::widget_for_property(model::Property *property)
{
    if (!property)
        return nullptr;

    auto type = property->type();
    switch (type) {
    case alive::model::PropertyType::e_StaticSingleLineText: {
        auto *p = model::to_static_property<std::string>(property);
        QLineEdit *le = new QLineEdit(QString::fromStdString(p->get_value()));
        le->connect(le, &QLineEdit::textChanged, le, [p](const QString &text) {
            p->set_value(text.toUtf8().toStdString());
        });
        return le;
    } break;
    case alive::model::PropertyType::e_StaticColor3V: {
        auto *p = model::to_static_property<Color>(property);
        ColorLineEditor *color_editor = new ColorLineEditor();
        color_editor->set_color(gui::from_color(p->get_value()));
        color_editor->connect(color_editor,
                              &ColorLineEditor::color_changed,
                              color_editor,
                              [p](const QColor &color) { p->set_value(gui::to_color(color)); });
        return color_editor;
    } break;
    case alive::model::PropertyType::e_StaticColor4ub: {
        auto *p = model::to_static_property<Color4ub>(property);
        ColorLineEditor *color_editor = new ColorLineEditor();
        color_editor->show_alpha(true);
        color_editor->set_color(gui::from_color4ub(p->get_value()));
        color_editor->connect(color_editor,
                              &ColorLineEditor::color_changed,
                              color_editor,
                              [p](const QColor &color) { p->set_value(gui::to_color4ub(color)); });
        return color_editor;
    } break;
    case alive::model::PropertyType::e_StaticSizeFactor:
    case alive::model::PropertyType::e_StaticFloat:
    case alive::model::PropertyType::e_StaticPositiveFloat: {
        auto *p = model::to_static_property<Vec1D>(property);
        QDoubleSpinBox *double_editor = new QDoubleSpinBox();
        double_editor->setValue(p->get_value());
        if (type == alive::model::PropertyType::e_StaticSizeFactor) {
            double_editor->setMinimum(0.0);
            double_editor->setMaximum(1.0);
            double_editor->setSingleStep(0.01);
        } else if (type == alive::model::PropertyType::e_StaticPositiveFloat) {
            double_editor->setMinimum(0.0);
            double_editor->setSingleStep(0.01);
        }
        double_editor->connect(double_editor,
                               &QDoubleSpinBox::valueChanged,
                               double_editor,
                               [p](const double &value) {
                                   p->set_value(static_cast<Vec1D>(value));
                               });
        return double_editor;
    } break;
    case alive::model::PropertyType::e_Direction: {
        auto *p = model::to_static_property<int>(property);
        QComboBox *editor = new QComboBox();
        editor->addItems({"Left", "Right", "Top", "Bottom"});
        editor->setCurrentIndex(p->get_value());
        editor->connect(editor, &QComboBox::currentIndexChanged, editor, [p](int index) {
            p->set_value(index);
        });
        return editor;
    } break;
    case alive::model::PropertyType::e_Font: {
        auto *p = model::to_static_property<core::Font>(property);
        FontSettingWidget *font_editor = new FontSettingWidget();
        font_editor->set_font(p->get_value());
        font_editor->connect(font_editor,
                             &FontSettingWidget::font_changed,
                             font_editor,
                             [p](const core::Font &font) { p->set_value(font); });
        return font_editor;
    } break;
    case alive::model::PropertyType::e_Bool: {
        auto *p = model::to_static_property<bool>(property);
        QCheckBox *check_box = new QCheckBox();
        check_box->setChecked(p->get_value());
        auto set_val_text = [check_box]() {
            check_box->setText(check_box->isChecked() ? QObject::tr("True") : QObject::tr("False"));
        };
        set_val_text();
        check_box->connect(check_box, &QCheckBox::clicked, check_box, [p, set_val_text](bool val) {
            p->set_value(val);
            set_val_text();
        });
        return check_box;
    } break;
    default:
        break;
    }
    return nullptr;
}

} // namespace alive::property
