#ifndef CHECKABLEICON_H
#define CHECKABLEICON_H

#include <QIcon>
#include <QWidget>

namespace alive {
class CheckableIcon : public QWidget
{
    Q_OBJECT
public:
    explicit CheckableIcon(const QSize &size,
                           const QString &name,
                           QWidget *parent = nullptr,
                           bool checked = true);

    void set_checked(bool clicked);
    bool is_checked() const { return m_checked; }
signals:
    void checked(bool);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    bool m_checked = true;
    QIcon m_icon;
};
} // namespace alive
#endif // CHECKABLEICON_H
