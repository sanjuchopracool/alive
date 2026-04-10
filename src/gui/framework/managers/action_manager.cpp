#include "action_manager.h"
#include <algorithm>
#include <QAction>

namespace alive {

namespace {

static ActionManager *the_instance = nullptr;
const static QString k_empty_string;
const static std::vector<std::unique_ptr<ActionContextItem>> k_empty_items;
} // namespace

class ActionContext
{
public:
    explicit ActionContext(const char *id, const QString &name)
        : m_id(id)
        , m_name(name)
    {}

    ActionContextItem *add_context(ActionContext *context, ActionContextItem *parent)
    {
        if (context) {
            m_items.emplace_back(new ActionContextItem(context, parent));
            return m_items.back().get();
        }
        return nullptr;
    }

    void clear() { m_items.clear(); }
    void add_action(Action *action, ActionContextItem *parent)
    {
        if (action) {
            m_items.emplace_back(new ActionContextItem(action, parent));
        }
    }
    int index_of_item(const ActionContextItem *item) const
    {
        auto it = std::find_if(m_items.begin(),
                               m_items.end(),
                               [item](const std::unique_ptr<ActionContextItem> &p) {
                                   return p.get() == item;
                               });
        if (it != m_items.end()) {
            return it - m_items.begin();
        }
        return -1;
    }

    const QString &name() const { return m_name; }
    const AliveStrID &id() const { return m_id; }

private:
    AliveStrID m_id;
    const QString m_name;
    std::vector<std::unique_ptr<ActionContextItem>> m_items;
    friend class ActionManager;
    friend class ActionContextItem;
};

ActionManager::ActionManager(QObject *parent)
    : QAbstractItemModel(parent)
{}

void ActionManager::init()
{
    if (!the_instance) {
        the_instance = new ActionManager();
    }
}

void ActionManager::deinit(bool qml)
{
    if (the_instance) {
        delete the_instance;
        the_instance = nullptr;
    }
}

ActionManager *ActionManager::instance()
{
    return the_instance;
}

ActionContextItem *ActionManager::create_root_context(const char *context_id,
                                                      const QString &name,
                                                      RootContexts name_type)
{
    if (context_id) {
        ActionContext *exist = the_instance->context_for_id(context_id);
        Q_ASSERT_X(exist == nullptr,
                   "ActionContextItems::add_context",
                   " another context with same id already exist!");
        m_contexts.emplace_back(new ActionContextItem(context_id, name));

        if (name_type == e_App) {
            m_app_context = m_contexts.back().get();
        } else if (name_type == e_Panels) {
            m_panel_context = m_contexts.back().get();
        }
        return m_contexts.back().get();
    }
    return nullptr;
}

QModelIndex ActionManager::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        if (row < m_contexts.size())
            return createIndex(row, column, m_contexts[row].get());
    } else {
        const ActionContextItem *item = static_cast<const ActionContextItem *>(
            parent.internalPointer());
        if (item->is_context()) {
            return createIndex(row, column, item->context()->m_items[row].get());
        }
    }

    return QModelIndex();
}

QModelIndex ActionManager::parent(const QModelIndex &child) const
{
    if (child.internalPointer()) {
        const ActionContextItem *item = static_cast<const ActionContextItem *>(
            child.internalPointer());
        const ActionContextItem *parent = item->parent();
        if (parent) {
            const ActionContextItem *grand_parent = parent->parent();
            if (grand_parent) {
                return createIndex(grand_parent->index_of_item(parent), 0, parent);
            }
        }
    }

    return QModelIndex();
}

int ActionManager::columnCount(const QModelIndex &parent) const
{
    return 2;
}

int ActionManager::rowCount(const QModelIndex &parent) const
{
    int row_count = 0;
    if (!parent.isValid()) {
        row_count = m_contexts.size();
    } else {
        const ActionContextItem *item = static_cast<const ActionContextItem *>(
            parent.internalPointer());
        if (item->is_context()) {
            return item->context()->m_items.size();
        }
    }

    return row_count;
}

QVariant ActionManager::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {
        if (role == Qt::DisplayRole) {
            const ActionContextItem *item = static_cast<const ActionContextItem *>(
                index.internalPointer());
            if (item) {
                if (index.row() == 0) {
                    if (item->is_context()) {
                        return item->context()->m_name;
                    } else {
                        return item->action()->q_action()->data();
                    }
                }
            }
        }
    }
    return {};
}

Action *ActionManager::action_for_id(const char *id) const
{
    for (const auto &item : m_contexts) {
        auto action = action_for_id_in_context(item.get(), id);
        if (action) {
            return action;
        }
    }

    return nullptr;
}

ActionContext *ActionManager::context_for_id(const char *id) const
{
    for (const auto &item : m_contexts) {
        auto context = context_for_id_in_context(item.get(), id);
        if (context) {
            return context;
        }
    }

    return nullptr;
}

QList<QAction *> ActionManager::get_context_actions(const char *id)
{
    QList<QAction *> actions;
    ActionContext *context = context_for_id(id);
    if (context) {
        for (const auto &item : context->m_items) {
            if (!item->is_context()) {
                actions.append(item->action()->q_action());
            }
        }
    }
    return actions;
}

QList<QAction *> ActionManager::get_app_context_actions(const char *id)
{
    QList<QAction *> actions;
    if (!m_app_context)
        return actions;

    for (const auto &context : m_app_context->context_items()) {
        if (context->is_context() && context->context()->id() == id) {
            for (const auto &item : context->context_items()) {
                if (!item->is_context()) {
                    actions.append(item->action()->q_action());
                }
            }
        }
    }
    return actions;
}

QList<QAction *> ActionManager::get_panel_context_actions(const char *id)
{
    QList<QAction *> actions;
    if (!m_panel_context)
        return actions;

    for (const auto &context : m_panel_context->context_items()) {
        if (context->is_context() && context->context()->id() == id) {
            for (const auto &item : context->context_items()) {
                if (!item->is_context()) {
                    actions.append(item->action()->q_action());
                }
            }
        }
    }
    return actions;
}

Action *ActionManager::action_for_id_in_context(const ActionContextItem *context,
                                                const char *id) const
{
    if (context->is_context()) {
        for (const auto &item : context->context_items()) {
            auto action = action_for_id_in_context(item.get(), id);
            if (action) {
                return action;
            }
        }
    } else {
        if (context->action()->id() == id) {
            return context->action();
        }
    }

    return nullptr;
}

ActionContext *ActionManager::context_for_id_in_context(const ActionContextItem *context,
                                                        const char *id) const
{
    if (context->is_context()) {
        if (context->context()->id() == id)
            return context->context();

        for (const auto &item : context->context_items()) {
            auto context = context_for_id_in_context(item.get(), id);
            if (context) {
                return context;
            }
        }
    }

    return nullptr;
}

ActionContextItem::ActionContextItem(const char *context,
                                     const QString &name,
                                     ActionContextItem *parent)
    : m_context(new ActionContext(context, name))
    , m_is_context(true)
    , m_parent(parent)
{}

const QString &ActionContextItem::context_name() const
{
    if (m_is_context) {
        return m_context->name();
    } else {
        Q_ASSERT_X(false,
                   "ActionContextItems",
                   "context_name is valid for only context, not action");
    }
    return k_empty_string;
}

const ActionContextItems &ActionContextItem::context_items() const
{
    if (m_is_context) {
        return m_context->m_items;
    } else {
        Q_ASSERT_X(false,
                   "ActionContextItems",
                   "items only valid for Action context not for action");
    }
    return k_empty_items;
}

const AliveStrID ActionContextItem::id() const
{
    if (m_is_context) {
        m_context->id();
    }
    return m_action->id();
}

int ActionContextItem::index_of_item(const ActionContextItem *item) const
{
    if (m_is_context) {
        return m_context->index_of_item(item);
    } else {
        Q_ASSERT_X(false, "ActionContextItems", " can not search item in action");
    }
    return -1;
}

ActionContextItem::~ActionContextItem()
{
    if (m_is_context) {
        delete m_context;
    } else {
        delete m_action;
    }
}

ActionContextItem *ActionContextItem::add_context(const char *context, const QString &name)
{
    if (m_is_context) {
        ActionContext *exist = the_instance->context_for_id(context);
        Q_ASSERT_X(exist == nullptr,
                   "ActionContextItems::add_context",
                   " another context with same id already exist!");

        return m_context->add_context(new ActionContext(context, name), this);
    } else {
        Q_ASSERT_X(false, "ActionContextItems", " can not add item to action");
    }
    return nullptr;
}

void ActionContextItem::clear_items()
{
    if (m_is_context) {
        m_context->clear();
    }
}

void ActionContextItem::add_action(const char *action_id, QAction *action)
{
    if (m_is_context) {
        Action *exist = the_instance->action_for_id(action_id);
        Q_ASSERT_X(exist == nullptr,
                   "ActionContextItems::add_action",
                   " another action with same id already exist!");
        m_context->add_action(new Action(action_id, action), this);
    } else {
        Q_ASSERT_X(false, "ActionContextItems", " can not add item to action");
    }
}

} // namespace alive
