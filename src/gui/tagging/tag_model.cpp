#include "tag_model.h"
#include <core/tagging/tag_manager.h>
#include <core/tagging/taggable.h>
namespace alive {
TagModel::TagModel(TagManager *mgr, QObject *parent)
    : QAbstractItemModel{parent}
    , m_tag_manager(mgr)
{}

Qt::ItemFlags TagModel::flags(const QModelIndex &index) const
{
    auto flags = QAbstractItemModel::flags(index);
    if (!index.internalPointer()) {
        flags |= Qt::ItemFlag::ItemIsEditable;
    }
    return flags;
}

QModelIndex TagModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid()) {
        Tag *tag = m_tag_manager->at(parent.row());
        if (tag) {
            return createIndex(row, column, tag);
        }

        return {};
    }

    // no id for tags
    return createIndex(row, column);
}

QModelIndex TagModel::parent(const QModelIndex &child) const
{
    if (child.internalPointer()) {
        // taggable
        Tag *tag = static_cast<Tag *>(child.internalPointer());
        if (tag) {
            int index = m_tag_manager->index(tag);
            if (index >= 0) {
                return createIndex(index, 0);
            }
        }
    }

    return QModelIndex();
}

int TagModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        if (!parent.internalPointer()) {
            Tag *tag = m_tag_manager->at(parent.row());
            if (tag) {
                return tag->count();
            }
        }

        return 0;
    }

    return m_tag_manager->count();
}

int TagModel::columnCount(const QModelIndex &) const
{
    return 1;
}

QVariant TagModel::data(const QModelIndex &index, int role) const
{
    Taggable *taggable = nullptr;
    Tag *tag = nullptr;
    if (index.internalPointer()) {
        // taggable
        Tag *tag = static_cast<Tag *>(index.internalPointer());
        if (tag) {
            taggable = tag->at(index.row());
        }
    } else {
        tag = m_tag_manager->at(index.row());
    }

    if (role == Qt::DisplayRole) {
        if (tag) {
            return QString::fromStdString(tag->name());
        }

        if (taggable) {
            return QString::fromStdString(taggable->name());
        }
    }
    return {};
}

bool TagModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Tag *tag = m_tag_manager->at(index.row());
    if (tag && role == Qt::EditRole) {
        std::string name = value.toString().toStdString();
        if (!name.empty() && !m_tag_manager->tag(name)) {
            tag->set_name(name);
            return true;
        }
    }

    return false;
}

void alive::TagModel::tag(Taggable *to, Tag *tag)
{
    if (to && tag) {
        int index = m_tag_manager->index(tag);
        int count = tag->count();
        beginInsertRows(createIndex(index, 0), count, count);
        to->add_tag(tag);
        endInsertRows();
    }
}

void TagModel::remove_tag(Taggable *from, Tag *tag)
{
    if (from && tag) {
        int index = m_tag_manager->index(tag);
        beginRemoveRows(createIndex(index, 0), index, index);
        from->remove_tag(tag);
        endRemoveRows();
    }
}

bool TagModel::has_tag(const std::string &str)
{
    return m_tag_manager->tag(str);
}

Tag *TagModel::create_tag(const std::string &str)
{
    if (has_tag(str))
        return nullptr;

    int count = m_tag_manager->count();
    beginInsertRows({}, count, count);
    auto *tag = m_tag_manager->create_tag(str);
    endInsertRows();
    return tag;
}

bool TagModel::delete_tag(Tag::TagId id)
{
    int index = m_tag_manager->index(id);
    if (index == -1)
        return false;

    beginRemoveRows({}, index, index);
    bool result = m_tag_manager->delete_tag(id);
    endRemoveRows();
    return result;
}

std::vector<TagModel::TagDeleteContext> TagModel::from_selection(
    const QModelIndexList &selection_list)
{
    // get list of all items to be removed from tag
    // also get tags to be removed
    QMap<Tag *, TagModel::TagDeleteContext> selection;
    for (auto &index : selection_list) {
        Tag *tag = nullptr;
        if (!index.parent().isValid()) {
            // its a tag
            tag = m_tag_manager->at(index.row());
            if (tag) {
                selection[tag].delete_tag = true;
            }
        } else {
            if (index.internalPointer()) {
                // taggable
                Tag *tag = static_cast<Tag *>(index.internalPointer());
                if (tag) {
                    if (!selection[tag].delete_tag) {
                        Taggable *taggable = tag->at(index.row());
                        if (taggable) {
                            selection[tag].entities.emplace_back(taggable);
                        }
                    }
                }
            }
        }
    }

    std::vector<TagModel::TagDeleteContext> result;
    for (auto it = selection.begin(), end = selection.end(); it != end; ++it) {
        result.push_back(it.value());
        TagDeleteContext &context = result.back();
        context.tag_name = it.key()->name();
        if (context.delete_tag) {
            context.entities = it.key()->entities();
        }
    }
    return result;
}
} // namespace alive
