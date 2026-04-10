#ifndef TIMELINE_PLAYHEAD_H
#define TIMELINE_PLAYHEAD_H

#include <QWidget>

namespace alive {

class TimelinePlayHead : public QWidget
{
    Q_OBJECT
public:
    enum Type { e_Selected, e_Play };
    explicit TimelinePlayHead(QWidget *parent = nullptr, Type type = e_Selected);

signals:

protected:
    void paintEvent(QPaintEvent *ev) override;

private:
    Type m_type;
    QIcon m_playhead_icon;
};

} // namespace alive

#endif // TIMELINE_PLAYHEAD_H
