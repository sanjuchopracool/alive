#ifndef NUMBEREDITOR_H
#define NUMBEREDITOR_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QValidator;
class QLabel;
QT_END_NAMESPACE

namespace alive {
class LabelEditor;
class LineEditor;
class NumberEditor : public QWidget
{
    Q_OBJECT
public:
    explicit NumberEditor(QWidget *parent = nullptr, bool add_spacer = true);

    void set_suffix(const QString &suffix);

protected:
    void focusInEvent(QFocusEvent *event) override;

signals:
    void delta_changed(qreal delta);

private slots:
    void enable_line_edit();
    void enable_label_edit();

protected:
    virtual void calculate_delta(const QPointF &pos);
    void update_text();
    virtual QString text_value(uint8_t decimel = 0) const = 0;
    virtual void increase() = 0;
    virtual void decrease() = 0;
    virtual void set_value_from_text(const QString &text) = 0;

private:
    LabelEditor *m_label_edit = nullptr;
    LineEditor *m_line_edit = nullptr;
    QLabel *m_suffix_label = nullptr;
    QPointF m_prev_pos;
    qreal m_delta = 0.0f;
};

class IntEditor : public NumberEditor
{
    Q_OBJECT
public:
    explicit IntEditor(QWidget *parent = nullptr, bool add_spacer = true);

    void set_range(int min, int max);
    void set_value(int value);
    int value() const { return m_value; }

signals:
    void value_changed(int);

protected:
    QString text_value(uint8_t) const override;
    void increase() override;
    void decrease() override;
    void set_value_from_text(const QString &text) override;

private:
    int validated_value(int value) const;

private:
    int m_value = 0;
    int m_min = 0;
    int m_max = 100;
};

class DoubleEditor : public NumberEditor
{
    Q_OBJECT
public:
    explicit DoubleEditor(QWidget *parent = nullptr, bool add_spacer = true);

    void set_range(double min, double max);
    void set_value(double value);

    double value() const { return m_value; }

signals:
    void value_changed(double);

protected:
    QString text_value(uint8_t decimel) const override;
    void increase() override;
    void decrease() override;
    void set_value_from_text(const QString &text) override;

private:
    double validated_value(double value) const;

private:
    double m_value = 0;
    double m_min = 0;
    double m_max = 100;
};

class IntPairEditor : public QWidget
{
    Q_OBJECT
public:
    IntPairEditor(QWidget *parent);
    void set_suffix(const QString &suffix);
    void set_range(int min, int max);

    void set_x(int x);
    void set_y(int y);

    int x_value() const;
    int y_value() const;
signals:
    void x_changed(int);
    void y_changed(int);

private:
    IntEditor *m_left = nullptr;
    IntEditor *m_right = nullptr;
};

class DoublePairEditor : public QWidget
{
    Q_OBJECT
public:
    DoublePairEditor(QWidget *parent);
    void set_suffix(const QString &suffix);
    void set_range(double min, double max);

    void set_x(double x);
    void set_y(double y);

    double x_value() const;
    double y_value() const;

signals:
    void x_changed(double);
    void y_changed(double);

private:
    DoubleEditor *m_left = nullptr;
    DoubleEditor *m_right = nullptr;
};
} // namespace alive
#endif // NUMBEREDITOR_H
