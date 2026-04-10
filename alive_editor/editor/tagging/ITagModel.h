#ifndef ITAGMODEL_H
#define ITAGMODEL_H

#include <core/tagging/tag.h>

namespace alive
{
class Tag;
class Taggable;

class ITagModel
{
public:
    struct TagDeleteContext
    {
        bool delete_tag = false;
        std::string tag_name;
        std::vector<Taggable *> entities;
    };
    virtual void tag(Taggable *to, Tag *tag) = 0;
    virtual void remove_tag(Taggable *from, Tag *tag) = 0;
    virtual bool has_tag(const std::string &str) = 0;
    virtual Tag *create_tag(const std::string &str) = 0;
    virtual bool delete_tag(Tag::TagId id) = 0;
};

}

#endif // ITAGMODEL_H
