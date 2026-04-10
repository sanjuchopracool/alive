#ifndef TIMELINE_PROPERTY_EDITOR_H
#define TIMELINE_PROPERTY_EDITOR_H

#include <QWidget>

namespace alive {
class TimelinePropertyEditor : public QWidget
{
    Q_OBJECT

public:
    TimelinePropertyEditor(QWidget *parent = nullptr);
    ~TimelinePropertyEditor();
};
} // namespace Alive
#endif // TIMELINE_PROPERTY_EDITOR_H
