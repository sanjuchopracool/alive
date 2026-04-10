#ifndef EDIT_COLUMN_HEADER_H
#define EDIT_COLUMN_HEADER_H

#include <QWidget>

namespace alive {
class EditColumnHeader : public QWidget
{
    Q_OBJECT
public:
    explicit EditColumnHeader(QWidget *parent = nullptr);

    void set_show_matte_info(bool show);
    bool show_matte_info() const;
    int before_expand_width() const;
    int expanded_width() const;

protected:
    void paintEvent(QPaintEvent *ev) override;
    void mousePressEvent(QMouseEvent *ev) override;

signals:
    void show_matte_info_changed(bool, int);

private:
    bool m_show_matte_details = false;
    int m_old_width;
};
} // namespace alive

#endif // EDIT_COLUMN_HEADER_H
