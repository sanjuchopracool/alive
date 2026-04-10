#include "menubar.h"
#include <alive/action_ids.h>
#include <gui/framework/managers/action_manager.h>
#include <gui/managers/dock_action_manager.h>
#include <gui/project/project_manager.h>

#include <QApplication>
#include <QFileDialog>

namespace alive {

QMenu *recent_files_menu = nullptr;

MenuBar::MenuBar(QWidget *parent)
    : QMenuBar(parent)
{
    for (const auto &item : ActionManager::instance()->get_app_context()->context_items()) {
        if (item->is_context()) {
            add_menu_item(item.get(), nullptr);
        } else {
            addAction(item->action()->q_action());
        }
    }
}

void MenuBar::update_recent_menu(ActionContextItem *item)
{
    if (recent_files_menu && item) {
        recent_files_menu->clear();
        for (const auto &item : item->context_items()) {
            add_menu_item(item.get(), recent_files_menu);
        }
    }
}

QMenu *MenuBar::add_menu_item(ActionContextItem *item, QMenu *parent)
{
    if (item->is_context()) {
        QMenu *menu = new QMenu(item->context_name());

        if (!recent_files_menu && item->id() == action_ids::k_recent_files_key) {
            recent_files_menu = menu;
        }

        if (parent) {
            parent->addMenu(menu);
        } else {
            this->addMenu(menu);
        }
        for (const auto &item : item->context_items()) {
            add_menu_item(item.get(), menu);
        }
        return menu;
    } else {
        if (parent) {
            parent->addAction(item->action()->q_action());
        }
    }

    return nullptr;
}

} // namespace alive
