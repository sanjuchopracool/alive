#ifndef EDIT_COLUMN_DELEGATE_H
#define EDIT_COLUMN_DELEGATE_H

#include <memory>
#include <QAbstractItemDelegate>

namespace alive::theme {
class ColorPalette;
class TimelineSize;
} // namespace alive::theme

namespace alive::model {
class CompositionDocument;
}

namespace alive {
struct EditColumnDelegatePrivateData;
class EditColumnDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:
    explicit EditColumnDelegate(const alive::model::CompositionDocument *doc,
                                QAbstractItemModel *model,
                                QObject *parent = nullptr);
    ~EditColumnDelegate();

    void set_handling_timeline(bool);
    bool is_dragging() const;
    void stop_dragging();
    void drag(QPointF p);

    void request_editor(QModelIndex);
    void request_close_editor(QWidget *editor);

    bool eventFilter(QObject *obj, QEvent *event) override;

protected:
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    bool editorEvent(QEvent *event,
                     QAbstractItemModel *model,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;

    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

signals:
    void invoke_open_editor(const QModelIndex &);
    void invoke_close_editor(QWidget *);

private:
    void draw_property_text(const QVariant &value,
                            const QModelIndex &index,
                            QPainter *painter,
                            const QStyleOptionViewItem &option,
                            const alive::theme::ColorPalette *colors,
                            const alive::theme::TimelineSize *sizes) const;
    void draw_path_text(bool value,
                        QPainter *painter,
                        const QStyleOptionViewItem &option,
                        const alive::theme::ColorPalette *colors,
                        const alive::theme::TimelineSize *sizes) const;
    void draw_layer_properties(QVariant value,
                               QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const alive::theme::ColorPalette *colors,
                               const alive::theme::TimelineSize *sizes) const;

private:
    std::unique_ptr<EditColumnDelegatePrivateData> m_d;
};

} // namespace alive

#endif // EDIT_COLUMN_DELEGATE_H
