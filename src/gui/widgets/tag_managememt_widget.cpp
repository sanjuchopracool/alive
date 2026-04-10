#include "tag_managememt_widget.h"
#include <gui/document/composition_document.h>
#include <editor/undo_support/document_undo_manager.h>
#include <gui/project/project_manager.h>
#include <gui/tagging/tag_model.h>
#include <gui/widgets/utility.h>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QInputDialog>
#include <QKeyEvent>
#include <QPushButton>
#include <QTreeView>

namespace alive {
TagManagememtWidget::TagManagememtWidget(QWidget *parent)
    : QWidget{parent}
{
    auto *layout = vbox_layout();
    m_view = new QTreeView(this);
    layout->addWidget(m_view);
    auto *create_btn = new QPushButton(tr("Create Tag"), this);
    layout->addWidget(create_btn);
    setLayout(layout);

    // selection policy
    m_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
    connect(create_btn, &QPushButton::clicked, this, [this]() {
        QInputDialog dlg(this);
        dlg.setInputMode(QInputDialog::TextInput);
        dlg.setLabelText(tr("Tag Name"));
        dlg.setWindowTitle(tr("New Tag"));
        dlg.setVisible(true);
        auto *box = dlg.findChild<QDialogButtonBox *>();
        if (box) {
            auto *ok_btn = box->button(QDialogButtonBox::Ok);
            connect(&dlg, &QInputDialog::textValueChanged, &dlg, [ok_btn, &dlg, this]() {
                const auto &tag_name = dlg.textValue();
                ok_btn->setEnabled(!tag_name.isEmpty() && !m_model->has_tag(tag_name.toStdString()));
            });
        }

        int rtn_code = dlg.exec();
        const auto &tag_name = dlg.textValue();
        if (rtn_code == QInputDialog::Accepted && !tag_name.isEmpty()) {
            if (!m_model->has_tag(tag_name.toStdString())) {
                auto *doc = project::ProjectManager::instance()->current_document();
                if (doc) {
                    doc->undo_manager()->create_tag(tag_name.toStdString());
                }
            }
        }
    });

    m_view->header()->hide();
}

void TagManagememtWidget::set_tag_model(TagModel *model)
{
    if (model != m_model) {
        m_model = model;
        m_view->setModel(model);
    }
}

void TagManagememtWidget::keyPressEvent(QKeyEvent *ev)
{
    if (ev->key() == Qt::Key_Delete) {
        QModelIndexList selected_indexes = m_view->selectionModel()->selection().indexes();
        if (selected_indexes.size()) {
            auto *doc = project::ProjectManager::instance()->current_document();
            if (doc) {
                doc->undo_manager()->delete_with_context(m_model->from_selection(selected_indexes));
            }
        }
        ev->accept();
    }
    QWidget::keyPressEvent(ev);
}

} // namespace alive
