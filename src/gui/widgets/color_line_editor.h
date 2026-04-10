#ifndef COLOR_LINE_EDITOR_H
#define COLOR_LINE_EDITOR_H

#include <QLineEdit>
#include <QWidget>

namespace alive {
class ColorSelectionLabel;
class ColorLineEditor : public QWidget
{
    Q_OBJECT
public:
    explicit ColorLineEditor(QWidget *parent = nullptr);
    const QColor &color() const;

    void show_alpha(bool show);
public slots:
    void set_color(const QColor &color);

signals:
    void color_changed(QColor);

private:
    ColorSelectionLabel *m_label = nullptr;
    QLineEdit *m_edit = nullptr;
    bool m_show_alpha = false;
};
} // namespace alive

#endif // COLOR_LINE_EDITOR_H
