#ifndef LINKEDNUMBEREDITOR_H
#define LINKEDNUMBEREDITOR_H

#include <QWidget>

namespace alive {
class IntPairEditor;
class DoublePairEditor;
class CheckableIcon;

using IntPair = std::pair<int, int>;
using DoublePair = std::pair<double, double>;

class LinkedIntEditor : public QWidget
{
    Q_OBJECT
public:
    LinkedIntEditor(bool linked, QWidget *parent = nullptr);

    void set_suffix(const QString &suffix);
    void set_range(int min, int max);
    void set_value(const IntPair &value);
    void set_linked(bool link);

signals:
    void value_changed(IntPair);

private slots:
    void update_y(int y);
    void update_x(int x);

private:
    void update_ratio();

private:
    IntPairEditor *m_editor;
    CheckableIcon *m_icon;
    IntPair m_value = {};
    double m_ratio = 1;
};

class LinkedDoubleEditor : public QWidget
{
    Q_OBJECT
public:
    LinkedDoubleEditor(bool linked, QWidget *parent = nullptr);

    void set_suffix(const QString &suffix);
    void set_range(double min, double max);
    void set_value(const DoublePair &value);
    void set_linked(bool link);

signals:
    void value_changed(DoublePair);

private slots:
    void update_y(double y);
    void update_x(double x);

private:
    void update_ratio();

private:
    DoublePairEditor *m_editor;
    CheckableIcon *m_icon;
    DoublePair m_value = {};
    double m_ratio = 1;
};
} // namespace alive
#endif // LINKEDNUMBEREDITOR_H
