#ifndef ACTION_MANAGER_H
#define ACTION_MANAGER_H

#include <Corrade/Containers/String.h>
#include <memory>
#include <vector>
#include <QAbstractItemModel>
#include <QAction>

namespace alive {

using AliveStrID = Corrade::Containers::String;

class Action
{
public:
    using ActionShortcuts = QList<QKeySequence>;
    using ActionShortcut = QKeySequence;

    const AliveStrID id() const { return m_id; }
    const ActionShortcuts &default_shortcuts() const { return m_default_shortcuts; }
    ActionShortcuts shortcuts() const { return m_action->shortcuts(); }
    QAction *q_action() const { return m_action; }

private:
    Action(const char *id, QAction *action)
        : m_id(id)
        , m_action(action)
        , m_default_shortcuts(m_action->shortcuts())
    {}

    const AliveStrID m_id;
    QAction *m_action = nullptr;
    ActionShortcuts m_default_shortcuts;
    friend class ActionContextItem;
};

class ActionContext;
class ActionContextItem;
using ActionContextItems = std::vector<std::unique_ptr<ActionContextItem>>;

class ActionContextItem
{
public:
    bool is_context() const { return m_is_context; }
    Action *action() const { return m_is_context ? nullptr : m_action; }
    ActionContext *context() const { return m_is_context ? m_context : nullptr; }
    const QString &context_name() const;
    const ActionContextItems &context_items() const;
    const AliveStrID id() const;

    ActionContextItem *parent() const { return m_parent; }
    int index_of_item(const ActionContextItem *item) const;
    ~ActionContextItem();

    ActionContextItem *add_context(const char *context, const QString &name);
    void clear_items();

    /**
     * @brief add_action
     * @param action_id : must be unique
     * @param action
     */
    void add_action(const char *action_id, QAction *action);

private:
    explicit ActionContextItem(Action *action, ActionContextItem *parent)
        : m_action(action)
        , m_parent(parent)
    {}

    explicit ActionContextItem(ActionContext *c, ActionContextItem *parent = nullptr)
        : m_context(c)
        , m_is_context(true)
        , m_parent(parent)
    {}

    explicit ActionContextItem(const char *context,
                               const QString &name,
                               ActionContextItem *parent = nullptr);

    union {
        Action *m_action;
        ActionContext *m_context;
    };
    bool m_is_context = false;
    ActionContextItem *m_parent = nullptr;

    friend class ActionContext;
    friend class ActionManager;
};

class ActionManager : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum RootContexts {
        e_None,
        e_App,
        e_Panels,

    };
    ActionManager(QObject *parent = nullptr);

    // static instance related
    static void init();
    static void deinit(bool qml = false);
    static ActionManager *instance();

    // ActionManager will take ownership of context_id
    ActionContextItem *create_root_context(const char *context_id,
                                           const QString &name,
                                           RootContexts name_type = e_None);

    // AbstractItemModel
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Action *action_for_id(const char *id) const;
    ActionContext *context_for_id(const char *id) const;

    QList<QAction *> get_context_actions(const char *id);

    QList<QAction *> get_app_context_actions(const char *id);
    QList<QAction *> get_panel_context_actions(const char *id);

    const ActionContextItem *get_app_context() const { return m_app_context; }

private:
    Action *action_for_id_in_context(const ActionContextItem *context, const char *id) const;
    ActionContext *context_for_id_in_context(const ActionContextItem *context, const char *id) const;

private:
    std::vector<std::unique_ptr<ActionContextItem>> m_contexts;
    ActionContextItem *m_app_context = nullptr;
    ActionContextItem *m_panel_context = nullptr;
};

} // namespace alive

#endif // ACTION_MANAGER_H
