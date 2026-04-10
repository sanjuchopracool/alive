#include "property_widget_base.h"
#include "property_widget_factory.h"
#include <core/model/property/property.h>
#include <gui/widgets/utility.h>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>

namespace alive::property {
PropertyWidgetBase::PropertyWidgetBase(model::Property *prop,
                                       bool enable_name_editing,
                                       QWidget *parent)
    : QWidget(parent)
    , m_property(prop)
{
    m_layout = new QVBoxLayout();
    const auto &name = QString::fromStdString(prop->name());
    const auto &id = QString::fromStdString(prop->id());
    if (enable_name_editing) {
        auto *layout = new QHBoxLayout();
        auto *editor = new QLineEdit(name, this);
        auto *id_editor = new QLineEdit(id, this);
        layout->addWidget(editor);
        layout->addWidget(id_editor);
        connect(editor, &QLineEdit::textChanged, this, [this](const QString &str) {
            m_property->set_name(str.toStdString());
        });
        connect(id_editor, &QLineEdit::textChanged, this, [this](const QString &str) {
            m_property->set_id(str.toStdString());
        });

        auto create_tool_button = [](const QString &icon) {
            QToolButton *btn = new QToolButton;
            btn->setCheckable(false);
            // btn->setIconSize(QSize(20, 20));
            btn->setIcon(QIcon::fromTheme(icon));
            return btn;
        };

        auto *del_btn = create_tool_button("delete");
        auto *duplicate_btn = create_tool_button("duplicate");
        auto *up_btn = create_tool_button("up-arrow");
        connect(del_btn, &QToolButton::clicked, this, [this, prop]() { emit delete_clicked(prop); });
        connect(duplicate_btn, &QToolButton::clicked, this, [this, prop]() {
            emit duplicate_clicked(prop);
        });
        connect(up_btn, &QToolButton::clicked, this, [this, prop]() { emit up_clicked(prop); });

        layout->addWidget(del_btn);
        layout->addWidget(duplicate_btn);
        layout->addWidget(up_btn);
        m_layout->addLayout(layout);

    } else {
        auto *label = new QLabel(name, this);
        label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        m_layout->addWidget(label);
    }
    auto *w = PropertyWidgetFactory::widget_for_property(prop);
    if (w) {
        m_layout->addWidget(w);
    }
    setLayout(m_layout);
}

PropertyWidgetBase::~PropertyWidgetBase() {}
} // namespace alive::property
