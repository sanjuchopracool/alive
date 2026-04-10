#ifndef MENUBAR_H
#define MENUBAR_H

#include <QMenuBar>

namespace alive {

class ActionContextItem;
class MenuBar : public QMenuBar
{
    Q_OBJECT
public:
    MenuBar(QWidget *parent = nullptr);

    void update_recent_menu(ActionContextItem *item);

private:
    QMenu *add_menu_item(ActionContextItem *item, QMenu *parent);
};

} // namespace alive

#endif // MENUBAR_H
