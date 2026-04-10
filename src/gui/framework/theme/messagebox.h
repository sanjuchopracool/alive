#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include <QMessageBox>

namespace alive::theme {

class MessageBox
{
public:
    static int information(QWidget *parent,
                           const QString &title,
                           const QString &text,
                           QMessageBox::StandardButtons buttons = QMessageBox::Ok,
                           QMessageBox::StandardButton default_button = QMessageBox::NoButton);
    static int warning(QWidget *parent,
                       const QString &title,
                       const QString &text,
                       QMessageBox::StandardButtons buttons = QMessageBox::Ok,
                       QMessageBox::StandardButton default_button = QMessageBox::NoButton);
    static int error(QWidget *parent,
                     const QString &title,
                     const QString &text,
                     QMessageBox::StandardButtons buttons = QMessageBox::Ok,
                     QMessageBox::StandardButton default_button = QMessageBox::NoButton);
    static int critical(QWidget *parent,
                        const QString &title,
                        const QString &text,
                        QMessageBox::StandardButtons buttons = QMessageBox::Ok,
                        QMessageBox::StandardButton default_button = QMessageBox::NoButton);
};
} // namespace alive::theme
#endif // MESSAGEBOX_H
