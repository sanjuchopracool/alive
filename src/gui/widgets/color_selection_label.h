#ifndef COLOR_SELECTION_LABEL_H
#define COLOR_SELECTION_LABEL_H

#include <QWidget>

namespace alive {
class ColorSelectionLabel : public QWidget
{
    Q_OBJECT
public:
    explicit ColorSelectionLabel(QWidget *parent = nullptr);

    void set_color(const QColor &color);
    const QColor &color() const { return m_color; }

protected:
    QSize sizeHint() const override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *ev) override;

signals:
    void color_changed(QColor);
    void clicked();

private:
    QColor m_color = Qt::black;
};
} // namespace alive
#endif // COLOR_SELECTION_LABEL_H
