#include "number_editor.h"
#include <QApplication>
#include <QFocusEvent>
#include <QHBoxLayout>
#include <QStackedWidget>
#include "label_editor.h"
#include "line_editor.h"

namespace alive {
enum EditMode { eLabel, eLineEdit };
NumberEditor::NumberEditor(QWidget *parent, bool add_spacer)
    : QWidget{parent}
{
    m_label_edit = new LabelEditor(this);
    m_label_edit->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred));

    m_line_edit = new LineEditor(this);
    m_line_edit->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred));

    m_suffix_label = new QLabel(this);
    m_suffix_label->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred));

    QHBoxLayout *main_layout = new QHBoxLayout();
    main_layout->setContentsMargins(0, 0, 0, 0);
    main_layout->setSpacing(0);
    main_layout->addWidget(m_label_edit);
    main_layout->addSpacing(2);
    main_layout->addWidget(m_line_edit);
    main_layout->addWidget(m_suffix_label);

    if (add_spacer) {
        main_layout->addStretch();
    }
    setLayout(main_layout);

    connect(m_label_edit, &LabelEditor::mouse_released, this, &NumberEditor::enable_line_edit);
    connect(m_label_edit, &LabelEditor::mouse_move_started, this, [this](const QPointF &pos) {
        m_prev_pos = pos;
        update_text();
    });

    connect(m_label_edit, &LabelEditor::mouse_moved, this, [this](const QPointF &pos) {
        calculate_delta(pos);
        update_text();
    });

    connect(m_label_edit, &LabelEditor::mouse_move_end, this, [this](const QPointF &pos) {
        calculate_delta(pos);
        update_text();
    });

    connect(m_line_edit, &LineEditor::editing_ended, this, &NumberEditor::enable_label_edit);
    connect(m_line_edit, &LineEditor::up_key_pressed, this, [this]() {
        increase();
        update_text();
    });
    connect(m_line_edit, &LineEditor::down_key_pressed, this, [this]() {
        decrease();
        update_text();
    });
    connect(m_line_edit, &LineEditor::editing_finished, this, [this](const QString &str) {
        set_value_from_text(str);
    });

    setFocusPolicy(Qt::StrongFocus);
    setFixedHeight(m_line_edit->minimumSizeHint().height());
    m_line_edit->hide();
}

void NumberEditor::set_suffix(const QString &suffix)
{
    m_suffix_label->setText(suffix);
}

void NumberEditor::focusInEvent(QFocusEvent *event)
{
    if (m_line_edit->isVisible()) {
        m_line_edit->setFocus();
    } else {
        m_label_edit->setFocus();
    }
    event->accept();
}

void NumberEditor::enable_line_edit()
{
    m_line_edit->show();
    m_label_edit->hide();
    update_text();
}

void NumberEditor::enable_label_edit()
{
    m_label_edit->show();
    m_line_edit->hide();
    update_text();
}

void NumberEditor::calculate_delta(const QPointF &pos)
{
    m_delta = pos.x() - m_prev_pos.x() - (pos.y() - m_prev_pos.y());
    m_prev_pos = pos;
    emit delta_changed(m_delta);
}

void NumberEditor::update_text()
{
    if (m_line_edit->isVisible()) {
        m_line_edit->setText(text_value());
    } else {
        m_label_edit->setText(text_value(1));
    }
}

IntEditor::IntEditor(QWidget *parent, bool add_spacer)
    : NumberEditor(parent, add_spacer)
{
    connect(this, &NumberEditor::delta_changed, this, [this](qreal delta) {
        m_value = validated_value(m_value + delta);
        emit value_changed(m_value);
    });
    update_text();
}

void IntEditor::set_range(int min, int max)
{
    Q_ASSERT(max > min);
    m_min = min;
    m_max = max;
}

void IntEditor::set_value(int value)
{
    m_value = validated_value(value);
    update_text();
}

QString IntEditor::text_value(uint8_t) const
{
    return QString::number(m_value);
}

void IntEditor::increase()
{
    m_value = validated_value(m_value + 1);
    emit value_changed(m_value);
}

void IntEditor::decrease()
{
    m_value = validated_value(m_value - 1);
    emit value_changed(m_value);
}

void IntEditor::set_value_from_text(const QString &text)
{
    bool ok = false;
    int value = text.toInt(&ok);
    if (ok) {
        value = validated_value(value);
        if (m_value != value) {
            m_value = value;
            emit value_changed(m_value);
        }
    }
}

int IntEditor::validated_value(int value) const
{
    if (value < m_min)
        return m_min;
    if (value > m_max)
        return m_max;

    return value;
}

DoubleEditor::DoubleEditor(QWidget *parent, bool add_spacer)
    : NumberEditor(parent, add_spacer)
{
    connect(this, &NumberEditor::delta_changed, this, [this](qreal delta) {
        m_value = validated_value(m_value + delta);
        emit value_changed(m_value);
    });

    update_text();
}

void DoubleEditor::set_range(double min, double max)
{
    Q_ASSERT(max > min);
    m_min = min;
    m_max = max;
}

void DoubleEditor::set_value(double value)
{
    m_value = validated_value(value);
    update_text();
}

QString DoubleEditor::text_value(uint8_t decimel) const
{
    if (decimel)
        return QString::number(m_value, 'f', decimel);
    else
        return QString::number(m_value);
}

void DoubleEditor::increase()
{
    m_value = validated_value(m_value + 1);
    emit value_changed(m_value);
}

void DoubleEditor::decrease()
{
    m_value = validated_value(m_value - 1);
    emit value_changed(m_value);
}

void DoubleEditor::set_value_from_text(const QString &text)
{
    bool ok = false;
    double value = text.toDouble(&ok);
    if (ok) {
        value = validated_value(value);
        if (m_value != value) {
            m_value = value;
            emit value_changed(m_value);
        }
    }
}

double DoubleEditor::validated_value(double value) const
{
    if (value < m_min)
        return m_min;
    if (value > m_max)
        return m_max;

    return value;
}

IntPairEditor::IntPairEditor(QWidget *parent)
    : QWidget(parent)
{
    m_left = new IntEditor(this, false);
    m_right = new IntEditor(this);
    m_left->set_suffix(",");

    QHBoxLayout *main_layout = new QHBoxLayout();
    main_layout->setContentsMargins(0, 0, 0, 0);
    main_layout->setSpacing(0);
    main_layout->addWidget(m_left);
    main_layout->addWidget(m_right);
    setLayout(main_layout);

    connect(m_left, &IntEditor::value_changed, this, [this](int val) { emit x_changed(val); });
    connect(m_right, &IntEditor::value_changed, this, [this](int val) { emit y_changed(val); });
}

void IntPairEditor::set_suffix(const QString &suffix)
{
    m_right->set_suffix(suffix);
}

void IntPairEditor::set_range(int min, int max)
{
    m_left->set_range(min, max);
    m_right->set_range(min, max);
}

void IntPairEditor::set_x(int x)
{
    m_left->set_value(x);
}

void IntPairEditor::set_y(int y)
{
    m_right->set_value(y);
}

int IntPairEditor::x_value() const
{
    return m_left->value();
}

int IntPairEditor::y_value() const
{
    return m_right->value();
}

DoublePairEditor::DoublePairEditor(QWidget *parent)
    : QWidget(parent)
{
    m_left = new DoubleEditor(this, false);
    m_right = new DoubleEditor(this);
    m_left->set_suffix(",");

    QHBoxLayout *main_layout = new QHBoxLayout();
    main_layout->setContentsMargins(0, 0, 0, 0);
    main_layout->setSpacing(0);
    main_layout->addWidget(m_left);
    main_layout->addWidget(m_right);
    setLayout(main_layout);

    connect(m_left, &DoubleEditor::value_changed, this, [this](double val) { emit x_changed(val); });
    connect(m_right, &DoubleEditor::value_changed, this, [this](double val) {
        emit y_changed(val);
    });
}

void DoublePairEditor::set_suffix(const QString &suffix)
{
    m_right->set_suffix(suffix);
}

void DoublePairEditor::set_range(double min, double max)
{
    m_left->set_range(min, max);
    m_right->set_range(min, max);
}

void DoublePairEditor::set_x(double x)
{
    m_left->set_value(x);
}

void DoublePairEditor::set_y(double y)
{
    m_right->set_value(y);
}

double DoublePairEditor::x_value() const
{
    return m_left->value();
}

double DoublePairEditor::y_value() const
{
    return m_right->value();
}
} // namespace alive
