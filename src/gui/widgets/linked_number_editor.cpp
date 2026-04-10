#include "linked_number_editor.h"
#include "checkable_icon.h"
#include "number_editor.h"
#include <QHBoxLayout>

namespace alive {
LinkedIntEditor::LinkedIntEditor(bool linked, QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *main_layout = new QHBoxLayout();
    m_icon = new CheckableIcon(QSize(14, 14), "link", this, linked);
    m_editor = new IntPairEditor(this);
    main_layout->setContentsMargins(0, 0, 0, 0);
    main_layout->addWidget(m_icon);
    main_layout->addWidget(m_editor);
    main_layout->addStretch();
    setLayout(main_layout);

    connect(m_icon, &CheckableIcon::checked, this, [this](bool checked) {
        if (checked) {
            update_ratio();
        }
    });

    connect(m_editor, &IntPairEditor::x_changed, this, &LinkedIntEditor::update_x);
    connect(m_editor, &IntPairEditor::y_changed, this, &LinkedIntEditor::update_y);
}

void LinkedIntEditor::set_suffix(const QString &suffix)
{
    m_editor->set_suffix(suffix);
}

void LinkedIntEditor::set_range(int min, int max)
{
    m_editor->set_range(min, max);
}

void LinkedIntEditor::set_value(const IntPair &value)
{
    QSignalBlocker blk(this);
    m_editor->set_x(value.first);
    m_editor->set_y(value.second);
    m_value.first = m_editor->x_value();
    m_value.second = m_editor->y_value();
    update_ratio();
}

void LinkedIntEditor::set_linked(bool link)
{
    m_icon->set_checked(link);
}

void LinkedIntEditor::update_ratio()
{
    if (m_value.first && m_value.second) {
        m_ratio = static_cast<double>(m_value.first) / static_cast<double>(m_value.second);
    } else {
        m_ratio = 1;
    }
}

void LinkedIntEditor::update_y(int y)
{
    m_value.second = y;
    if (m_icon->is_checked()) {
        QSignalBlocker b(this);
        m_value.first = m_ratio * y;
        m_editor->set_x(m_value.first);
        m_value.first = m_editor->x_value();
        update_ratio();
    }
    emit value_changed({m_value.first, m_value.second});
}

void LinkedIntEditor::update_x(int x)
{
    m_value.first = x;
    if (m_icon->is_checked()) {
        QSignalBlocker b(this);
        m_value.second = x / m_ratio;
        m_editor->set_y(m_value.second);
        m_value.second = m_editor->y_value();
        update_ratio();
    }
    emit value_changed({m_value.first, m_value.second});
}

LinkedDoubleEditor::LinkedDoubleEditor(bool linked, QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *main_layout = new QHBoxLayout();
    m_icon = new CheckableIcon(QSize(14, 14), "link", this, linked);
    m_editor = new DoublePairEditor(this);
    main_layout->setContentsMargins(0, 0, 0, 0);
    main_layout->addWidget(m_icon);
    main_layout->addWidget(m_editor);
    main_layout->addStretch();
    setLayout(main_layout);

    connect(m_icon, &CheckableIcon::checked, this, [this](bool checked) {
        if (checked) {
            update_ratio();
        }
    });

    connect(m_editor, &DoublePairEditor::x_changed, this, &LinkedDoubleEditor::update_x);
    connect(m_editor, &DoublePairEditor::y_changed, this, &LinkedDoubleEditor::update_y);
}

void LinkedDoubleEditor::set_suffix(const QString &suffix)
{
    m_editor->set_suffix(suffix);
}

void LinkedDoubleEditor::set_range(double min, double max)
{
    m_editor->set_range(min, max);
}

void LinkedDoubleEditor::set_value(const DoublePair &value)
{
    QSignalBlocker blk(this);
    m_editor->set_x(value.first);
    m_editor->set_y(value.second);
    m_value.first = m_editor->x_value();
    m_value.second = m_editor->y_value();
    update_ratio();
}

void LinkedDoubleEditor::set_linked(bool link)
{
    m_icon->set_checked(link);
}

void LinkedDoubleEditor::update_ratio()
{
    if (m_value.first && m_value.second) {
        m_ratio = m_value.first / m_value.second;
    } else {
        m_ratio = 1;
    }
}

void LinkedDoubleEditor::update_y(double y)
{
    m_value.second = y;
    if (m_icon->is_checked()) {
        QSignalBlocker b(this);
        m_value.first = m_ratio * y;
        m_editor->set_x(m_value.first);
        m_value.first = m_editor->x_value();
        update_ratio();
    }
}

void LinkedDoubleEditor::update_x(double x)
{
    m_value.first = x;
    if (m_icon->is_checked()) {
        QSignalBlocker b(this);
        m_value.second = x / m_ratio;
        m_editor->set_y(m_value.second);
        m_value.second = m_editor->y_value();
        update_ratio();
    }
}
} // namespace alive
