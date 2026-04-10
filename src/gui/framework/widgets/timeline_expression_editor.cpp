#include "timeline_expression_editor.h"
#include <core/model/keyframes/keyframe.h>
#include <core/model/property/property.h>
#include <core/script/expression_engine_interface.h>
#include <gui/framework/theme/color_palette.h>
#include <gui/framework/theme/messagebox.h>
#include <gui/project/project_manager.h>
#include <gui/widgets/utility.h>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPushButton>
#include <QResizeEvent>
#include <QTextEdit>
#include <QVBoxLayout>

namespace alive {

TimelineExpressionEditor::TimelineExpressionEditor(QWidget *parent)
    : QWidget{parent}
{
    setFocusPolicy(Qt::StrongFocus);

    QVBoxLayout* layout = vbox_layout();
    m_text_edit = new QTextEdit(this);
    m_text_edit->setBackgroundRole(QPalette::Dark);
    layout->setContentsMargins(50, 0 , 50, 0);
    layout->addWidget(m_text_edit);
    setLayout(layout);
    m_apply_btn = new QPushButton(tr("Apply"), this);

    connect(m_apply_btn, &QPushButton::clicked, this, [this]() {
        // set and enable expression
        const auto&  expr = m_text_edit->toPlainText();
        auto prj_mgr = project::ProjectManager::instance();

        try {
            if (m_keyframe) {
                if (prj_mgr->is_valid_expression(m_keyframe->property(), expr)) {
                    emit set_keyframe_expression(m_keyframe, expr);
                }
            } else if (m_property) {
                if (prj_mgr->is_valid_expression(m_property, expr)) {
                    emit set_property_expression(m_property, expr);
                }
            } else {
                prj_mgr->is_valid_playground_expression(expr);
                emit playground_expression_changed(expr);
            }
        } catch (const script::ExpressionEngineInterface::Exception &e) {
            QString msg;
            if (e == script::ExpressionEngineInterface::Exception::e_Recursive) {
                msg = tr("Recursive expression!");
            } else {
                msg = tr("Invalid expression!");
            }
            theme::MessageBox::error(this, tr("Unable to set Expression"), msg);
        }
    });
}

void TimelineExpressionEditor::open_editor(model::KeyFrame *keyframe)
{
    if (keyframe) {
        m_keyframe = keyframe;
        m_property = nullptr;
        m_text_edit->setText(QString::fromStdString(m_keyframe->expression()));
    }
}

void TimelineExpressionEditor::open_editor(model::Property *property)
{
    if (property) {
        m_keyframe = nullptr;
        m_property = property;
        m_text_edit->setText(QString::fromStdString(property->get_expression(0)));
    }
}

void TimelineExpressionEditor::open_editor(QString &&text)
{
    m_keyframe = nullptr;
    m_property = nullptr;
    m_text_edit->setText(text);
}

void TimelineExpressionEditor::keyPressEvent(QKeyEvent *ev)
{
    if (ev->key() == Qt::Key_Escape && ev->modifiers() == Qt::NoModifier) {
        this->hide();
        ev->accept();
        return;
    }
    QWidget::keyPressEvent(ev);
}

void TimelineExpressionEditor::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    auto size = event->size();
    auto geo = m_apply_btn->geometry();
    geo.moveRight(size.width() - 50);
    geo.moveBottom(size.height());
    m_apply_btn->setGeometry(geo);
}

} // namespace alive
