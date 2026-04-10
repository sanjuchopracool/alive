#include "line_editor.h"

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>

namespace alive {
LineEditor::LineEditor(QWidget *parent)
    : QLineEdit(parent)
{
    connect(this, &QLineEdit::editingFinished, this, [this]() { emit editing_finished(text()); });
    connect(this, &QLineEdit::editingFinished, this, &LineEditor::editing_ended);
}

void LineEditor::focusOutEvent(QFocusEvent *event)
{
    emit editing_ended();
    event->accept();
}

void LineEditor::keyPressEvent(QKeyEvent *event)
{
    int key = event->key();
    if (key == Qt::Key_Escape) {
        emit editing_ended();
        event->accept();
    } else if (key == Qt::Key_Up) {
        emit up_key_pressed();
        event->accept();
    } else if (key == Qt::Key_Down) {
        emit down_key_pressed();
        event->accept();
    } else {
        QLineEdit::keyPressEvent(event);
    }
}
} // namespace alive
