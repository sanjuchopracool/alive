#ifndef TIMELINE_EXPRESSION_EDITOR_H
#define TIMELINE_EXPRESSION_EDITOR_H

#include <QWidget>

QT_FORWARD_DECLARE_CLASS(QTextEdit)
QT_FORWARD_DECLARE_CLASS(QPushButton)

namespace alive::model {
class KeyFrame;
class Property;
}

namespace alive {
class TimelineExpressionEditor : public QWidget
{
    Q_OBJECT
public:
    explicit TimelineExpressionEditor(QWidget *parent = nullptr);

    void open_editor(alive::model::KeyFrame *keyframe);
    void open_editor(alive::model::Property *property);
    void open_editor(QString &&text);

protected:
    void keyPressEvent(QKeyEvent *ev) override;
    void resizeEvent(QResizeEvent* event) override;

signals:
    void set_keyframe_expression(alive::model::KeyFrame *keyframe, const QString expr);
    void set_property_expression(alive::model::Property *property, const QString expr);
    void playground_expression_changed(QString expr);

private:
    alive::model::KeyFrame *m_keyframe = nullptr;
    alive::model::Property *m_property = nullptr;
    QTextEdit* m_text_edit = nullptr;
    QPushButton* m_apply_btn = nullptr;
};
} // namespace alive

#endif // TIMELINE_EXPRESSION_EDITOR_H
