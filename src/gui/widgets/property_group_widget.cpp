#include "property_group_widget.h"
#include <core/model/composition.h>
#include <core/model/property/property_group.h>
#include <gui/document/composition_document.h>
#include <editor/undo_support/document_undo_manager.h>
#include <gui/framework/widgets/new_property_dialog.h>
#include <gui/framework/widgets/property_editor/property_widget_base.h>
#include <gui/framework/widgets/script_editor_dialog.h>
#include <gui/project/project_manager.h>
#include <gui/widgets/utility.h>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QToolButton>

namespace alive {
PropertyGroupWidget::PropertyGroupWidget(QWidget *parent)
    : QWidget{parent}
{
    auto *layout = vbox_layout();
    m_view = new QListWidget(this);
    m_update_script_button = new QPushButton(tr("Update Apply Scrpt"));

    m_add_new_property = new QToolButton;
    m_add_new_property->setCheckable(false);
    // m_add_new_property->setIconSize(QSize(20, 20));
    m_add_new_property->setIcon(QIcon::fromTheme("add"));

    layout->addWidget(m_view);
    auto *h_layout = hbox_layout();
    h_layout->addWidget(m_add_new_property);
    h_layout->addWidget(m_update_script_button);
    layout->addLayout(h_layout);

    connect(m_update_script_button, &QPushButton::clicked, this, [this]() {
        auto doc = alive::project::ProjectManager::instance()->current_document();
        if (doc) {
            ScriptEditorDialog dlg(doc->composition()->apply_script());
            if (dlg.exec() == QDialog::Accepted) {
                doc->undo_manager()->set_apply_script(dlg.script_text());
            }
        }
    });

    connect(m_add_new_property, &QToolButton::clicked, this, [this]() {
        NewPropertyDialog dlg;
        if (dlg.exec() == QDialog::Accepted) {
            if (m_doc && m_group) {
                const auto &[type, name, id] = dlg.selected_data();
                m_doc->undo_manager()->add_property(m_group, type, name, id);
            }
        }
    });
    setLayout(layout);
}

void PropertyGroupWidget::populate(model::CompositionDocument *doc,
                                   model::PropertyGroup *group,
                                   bool enable_editing)
{
    if (!group || group != m_group) {
        m_view->clear();
    }

    if (group != m_group && doc) {
        m_group = group;
        m_doc = doc;
        m_enable_editing = enable_editing;
        m_update_script_button->setVisible(m_enable_editing);
        m_add_new_property->setVisible(m_enable_editing);

        connect(m_doc,
                &model::CompositionDocument::property_added,
                this,
                &PropertyGroupWidget::on_property_added,
                Qt::UniqueConnection);
        connect(m_doc,
                &model::CompositionDocument::property_removed,
                this,
                &PropertyGroupWidget::on_property_delete,
                Qt::UniqueConnection);
        auto count = m_group->count();
        for (size_t i = 0; i < count; ++i) {
            add_editor_for_property(m_group->at(i), i);
        }
    }
}

void PropertyGroupWidget::on_property_delete(model::PropertyGroup *group,
                                             model::Property *prop,
                                             int index)
{
    if (m_group == group && QObject::sender() == m_doc) {
        auto w = m_view->takeItem(index);
        if (w) {
            delete w;
        }
    }
}

void PropertyGroupWidget::on_property_added(model::PropertyGroup *group,
                                            model::Property *prop,
                                            int index)
{
    if (m_group == group && QObject::sender() == m_doc) {
        add_editor_for_property(prop, index);
    }
}

void PropertyGroupWidget::add_editor_for_property(model::Property *prop, int index)
{
    if (!prop)
        return;

    QListWidgetItem *newItem = new QListWidgetItem;
    auto *editor = new alive::property::PropertyWidgetBase(prop, m_enable_editing, this);
    newItem->setSizeHint(editor->sizeHint());
    m_view->insertItem(index, newItem);
    m_view->setItemWidget(newItem, editor);

    connect(editor,
            &property::PropertyWidgetBase::delete_clicked,
            this,
            [this](model::Property *prop) {
                m_doc->undo_manager()->delete_property(m_group, prop);
            });
    connect(editor,
            &property::PropertyWidgetBase::duplicate_clicked,
            this,
            [this](model::Property *prop) {
                m_doc->undo_manager()->duplicate_property(m_group, prop);
            });
    connect(editor, &property::PropertyWidgetBase::up_clicked, this, [this](model::Property *prop) {
        m_doc->undo_manager()->move_up_property(m_group, prop);
    });
}
} // namespace alive
