#ifndef TEXTPROPERTY_WIDGET_H
#define TEXTPROPERTY_WIDGET_H

#include <core/model/text_primitives/text.h>
#include <QToolButton>

namespace Ui {
class TextPropertyWidget;
}

namespace alive {

class ExclusiveToolButtonGroup;

namespace core {
class Text;
}

class TextPropertyWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TextPropertyWidget(QWidget *parent = nullptr);
    ~TextPropertyWidget();

    void update_from(const core::Text &text);
    void reset();

private:
    void update_ui();

signals:
    void text_changed(const core::Text& text);

private:
    Ui::TextPropertyWidget *m_ui;
    core::Text m_text;

    QToolButton* m_butt_cap_button = nullptr;
    QToolButton* m_round_cap_button = nullptr;
    QToolButton* m_square_cap_button = nullptr;
    QToolButton* m_bevel_join_button = nullptr;
    QToolButton* m_miter_join_button = nullptr;
    QToolButton* m_round_join_button = nullptr;

    ExclusiveToolButtonGroup* m_cap_group = nullptr;
    ExclusiveToolButtonGroup* m_join_group = nullptr;
};
} // namespace alive

#endif // TEXTPROPERTY_WIDGET_H
