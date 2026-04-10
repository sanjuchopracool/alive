#include "messagebox.h"
#include <gui/icon_manager.h>

namespace alive::theme {

enum class MsgIcon { e_Information, e_Warning, e_Error, e_Critical };

int message_box(QWidget *parent,
                const QString &title,
                const QString &text,
                QMessageBox::StandardButtons buttons,
                QMessageBox::StandardButton default_button,
                QMessageBox::Icon e_icon,
                const QIcon &icon)
{
    QMessageBox msg_box(parent);
    msg_box.setWindowTitle(title);
    msg_box.setText(text);
    msg_box.setStandardButtons(buttons);
    msg_box.setDefaultButton(default_button);
    msg_box.setIcon(e_icon);
#ifdef __APPLE__
    msg_box.setIconPixmap(icon.pixmap(QSize(64, 64)));
#endif
    return msg_box.exec();
}

int MessageBox::information(QWidget *parent,
                            const QString &title,
                            const QString &text,
                            QMessageBox::StandardButtons buttons,
                            QMessageBox::StandardButton default_button)
{
    return message_box(parent,
                       title,
                       text,
                       buttons,
                       default_button,
                       QMessageBox::Information,
                       IconManager::instance()->get_icon(IconManager::e_MsgBoxInformation));
}
int MessageBox::warning(QWidget *parent,
                        const QString &title,
                        const QString &text,
                        QMessageBox::StandardButtons buttons,
                        QMessageBox::StandardButton default_button)
{
    return message_box(parent,
                       title,
                       text,
                       buttons,
                       default_button,
                       QMessageBox::Warning,
                       IconManager::instance()->get_icon(IconManager::e_MsgBoxWarning));
}
int MessageBox::error(QWidget *parent,
                      const QString &title,
                      const QString &text,
                      QMessageBox::StandardButtons buttons,
                      QMessageBox::StandardButton default_button)
{
    return message_box(parent,
                       title,
                       text,
                       buttons,
                       default_button,
                       QMessageBox::Question,
                       IconManager::instance()->get_icon(IconManager::e_MsgBoxError));
}
int MessageBox::critical(QWidget *parent,
                         const QString &title,
                         const QString &text,
                         QMessageBox::StandardButtons buttons,
                         QMessageBox::StandardButton default_button)
{
    return message_box(parent,
                       title,
                       text,
                       buttons,
                       default_button,
                       QMessageBox::Icon::Critical,
                       IconManager::instance()->get_icon(IconManager::e_MsgBoxCritical));
}

} // namespace alive::theme
