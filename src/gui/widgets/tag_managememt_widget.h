#ifndef TAG_MANAGEMEMT_WIDGET_H
#define TAG_MANAGEMEMT_WIDGET_H

#include <QWidget>

QT_FORWARD_DECLARE_CLASS(QTreeView)

namespace alive {
class TagModel;
class TagManagememtWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TagManagememtWidget(QWidget *parent = nullptr);
    void set_tag_model(TagModel *model);

protected:
    void keyPressEvent(QKeyEvent *ev) override;

private:
    TagModel *m_model = nullptr;
    QTreeView *m_view = nullptr;
};
} // namespace alive

#endif // TAG_MANAGEMEMT_WIDGET_H
