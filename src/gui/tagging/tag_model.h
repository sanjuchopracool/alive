#ifndef TAG_MODEL_H
#define TAG_MODEL_H

#include <QAbstractItemModel>
#include <editor/tagging/ITagModel.h>

namespace alive {

class TagManager;
class Taggable;
class TagModel : public QAbstractItemModel, public ITagModel
{
    Q_OBJECT
public:
    explicit TagModel(TagManager *mgr, QObject *parent = nullptr);

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    void tag(Taggable *to, Tag *tag) override;
    void remove_tag(Taggable *from, Tag *tag) override;
    bool has_tag(const std::string &str) override;
    Tag *create_tag(const std::string &str) override;
    bool delete_tag(Tag::TagId id) override;

    std::vector<TagModel::TagDeleteContext> from_selection(const QModelIndexList &selection_list);

private:
    TagManager *m_tag_manager;
};

} // namespace alive
#endif // TAG_MODEL_H
