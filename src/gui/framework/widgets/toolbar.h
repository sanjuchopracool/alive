#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <QWidget>

namespace alive {

class ToolButton : public QWidget
{
    Q_OBJECT
public:
    explicit ToolButton(const QString &name, const QIcon &icon, QWidget *parent = nullptr);
    void select(bool select);
    const QString &name() const;

signals:
    void selected(ToolButton *, bool);

protected:
    QSize sizeHint() const override;
    void paintEvent(QPaintEvent *ev) override;

    void enterEvent(QEnterEvent *ev) override;
    void leaveEvent(QEvent *ev) override;

    void mousePressEvent(QMouseEvent *ev) override;

private:
    QString m_name;
    bool m_selected = false;

    QPixmap m_normal_pixmap;
    QPixmap m_active_pixmap;
};

class ToolBar : public QWidget
{
    Q_OBJECT
public:
    explicit ToolBar(Qt::Orientation direction, QWidget *parent = nullptr);
    QSize sizeHint() const override;

    void add_toolbutton(ToolButton *btn);

    ToolButton *active_button() const { return m_selected; }
signals:
    void selected(ToolButton *, bool);

protected:
    void paintEvent(QPaintEvent *ev) override;

private slots:
    void on_toolbutton_selected(ToolButton *, bool);

private:
    Qt::Orientation m_direction;
    QLayout *m_layout = nullptr;
    ToolButton *m_selected = nullptr;
};

} // namespace alive
#endif // TOOLBAR_H
