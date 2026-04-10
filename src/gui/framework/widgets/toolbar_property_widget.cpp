#include "toolbar_property_widget.h"
#include "toolbar.h"
#include <gui/settings/user_ui_setting.h>
#include <gui/tagging/tag_model.h>
#include <gui/widgets/property_group_widget.h>
#include <gui/widgets/tag_managememt_widget.h>
#include <gui/widgets/textproperty_widget.h>
#include <gui/widgets/utility.h>
#include <map>
#include <QHBoxLayout>
#include <QTabBar>
#include <QTreeView>

namespace alive {

struct ToolBarPropertyWidgetPrivateData
{
    ToolBar *bar = nullptr;
    QLayout *layout = nullptr;

    std::map<ToolButton *, std::pair<QWidget *, ToolBarContextType>> toolbar_to_widgets;
    std::map<ToolBarContextType, ToolButton *> type_to_button;
    QWidget *active_editor = nullptr;
    QTabWidget *tab = nullptr;
    int expanded_width = 250;

    // editors
    TextPropertyWidget *text_editor = nullptr;
    PropertyGroupWidget *property_editor = nullptr;
    TagManagememtWidget *tag_widget = nullptr;
    QWidget *null_widget = nullptr;
    int supported_contexts = 0;

    void set_support(ToolBarContextType type) { supported_contexts |= static_cast<int>(type); }
    void clear_support(ToolBarContextType type) { supported_contexts &= ~(static_cast<int>(type)); }
    bool check_support(ToolBarContextType type)
    {
        return supported_contexts & (static_cast<int>(type));
    }

    void update_current_widget()
    {
        if (bar->active_button()) {
            auto editor_it = toolbar_to_widgets.find(bar->active_button());
            if (editor_it != toolbar_to_widgets.end()) {
                if (check_support(editor_it->second.second) && editor_it->second.first) {
                    tab->setCurrentWidget(editor_it->second.first);
                    return;
                }
            }
        }
        tab->setCurrentWidget(null_widget);
    }
};

ToolBarPropertyWidget::ToolBarPropertyWidget(QWidget *parent)
    : QWidget{parent}
    , m_d(Corrade::Containers::pointer<alive::ToolBarPropertyWidgetPrivateData>())
{
    m_d->bar = new ToolBar(Qt::Vertical, this);
    m_d->tab = new QTabWidget(this);
    m_d->tab->setContentsMargins(0, 0, 0, 0);
    m_d->tab->tabBar()->hide();

    auto *layout = hbox_layout();
    layout->addWidget(m_d->tab);
    layout->addWidget(m_d->bar);
    m_d->layout = layout;

    m_d->tab->hide();

    m_d->tab->addTab(m_d->null_widget = new QWidget(parent), {});
    auto add_toolbar_widget =
        [this](ToolBarContextType type, const QString &name, const QString &icon, QWidget *w) {
            ToolButton *btn = new ToolButton(name, QIcon::fromTheme(icon));
            w->setEnabled(false);
            m_d->tab->addTab(w, btn->name());
            m_d->toolbar_to_widgets.emplace(btn, std::make_pair(w, type));
            m_d->type_to_button.emplace(type, btn);
            m_d->bar->add_toolbutton(btn);
        };
    // text tool
    {
        m_d->text_editor = new TextPropertyWidget();
        add_toolbar_widget(ToolBarContextType::e_Text,
                           tr("Text"),
                           "toolbutton_text",
                           m_d->text_editor);
        connect(m_d->text_editor,
                &TextPropertyWidget::text_changed,
                this,
                [this](const core::Text &text) {
                    emit text_changed(text);
                });
    }
    // property editor
    {
        m_d->property_editor = new PropertyGroupWidget();
        add_toolbar_widget(ToolBarContextType::e_Property,
                           tr("Property"),
                           "toolbutton_property",
                           m_d->property_editor);
    }

    // tagging
    {
        m_d->tag_widget = new TagManagememtWidget();
        add_toolbar_widget(ToolBarContextType::e_Tagging, tr("Tag"), "label", m_d->tag_widget);
    }

    setLayout(layout);

    // connect signals
    connect(m_d->bar, &ToolBar::selected, this, [this](ToolButton *btn, bool selected) {
        if (selected) {
            auto it = m_d->toolbar_to_widgets.find(btn);
            QWidget *editor = m_d->null_widget;
            if (it != m_d->toolbar_to_widgets.end()) {
                if (m_d->check_support(it->second.second)) {
                    editor = it->second.first;
                }

                if (editor) {
                    m_d->tab->setCurrentWidget(editor);
                    m_d->tab->show();
                    emit toolbar_visibility_changed(m_d->expanded_width);
                }
            }
        } else {
            m_d->expanded_width = width();
            m_d->tab->hide();
            emit toolbar_visibility_changed(m_d->bar->width());
        }
    });
}

ToolBarPropertyWidget::~ToolBarPropertyWidget() {}

int ToolBarPropertyWidget::expanded_width() const
{
    return m_d->expanded_width;
}

void ToolBarPropertyWidget::update_expanded_width()
{
    if (width() > m_d->bar->width()) {
        m_d->expanded_width = width();
    }
}

void ToolBarPropertyWidget::enable_text(bool enable)
{
    if (enable) {
        m_d->set_support(ToolBarContextType::e_Text);
    } else {
        m_d->clear_support(ToolBarContextType::e_Text);
    }

    if (m_d->text_editor) {
        m_d->text_editor->setEnabled(enable);
    }
}

void ToolBarPropertyWidget::set_text_properties(const core::Text &text)
{
    if (m_d->text_editor) {
        m_d->text_editor->update_from(text);
    }
}

void ToolBarPropertyWidget::set_property_group(model::CompositionDocument *doc,
                                               model::PropertyGroup *group,
                                               bool enable_editing)
{
    m_d->property_editor->setEnabled(group);
    if (group) {
        m_d->property_editor->populate(doc, group, enable_editing);
        m_d->set_support(ToolBarContextType::e_Property);
    } else {
        m_d->clear_support(ToolBarContextType::e_Property);
    }
}

void ToolBarPropertyWidget::update_current_widget()
{
    if (m_d->null_widget) {
        m_d->update_current_widget();
    }
}

void ToolBarPropertyWidget::set_tag_model(TagModel *model)
{
    m_d->tag_widget->setEnabled(model);
    if (model) {
        m_d->tag_widget->set_tag_model(model);
        m_d->set_support(ToolBarContextType::e_Tagging);
    } else {
        m_d->clear_support(ToolBarContextType::e_Tagging);
    }
}

void ToolBarPropertyWidget::restore_settings()
{
    QVariant state = alive::UserSettings::instance()->right_toolbar_state();
    if (state.isValid()) {
        QVariantList list = state.toList();
        if (list.size() >= 2) {
            ToolBarContextType type = static_cast<ToolBarContextType>(list[0].toInt());
            m_d->expanded_width = list[1].toInt();
            emit toolbar_visibility_changed(m_d->expanded_width);
            if (type != ToolBarContextType::e_None) {
                auto it = m_d->type_to_button.find(type);
                if (it != m_d->type_to_button.end()) {
                    it->second->select(true);
                }
            }
        }
    }
}

void ToolBarPropertyWidget::save_settings() const
{
    QVariantList state;
    auto *btn = m_d->bar->active_button();

    ToolBarContextType type = ToolBarContextType::e_None;
    if (btn) {
        auto it = m_d->toolbar_to_widgets.find(btn);
        if (it != m_d->toolbar_to_widgets.end()) {
            type = it->second.second;
        }
    }
    state.append(static_cast<int>(type));
    state.append(m_d->expanded_width);
    alive::UserSettings::instance()->save_right_toolbar_state(state);
}
} // namespace alive
