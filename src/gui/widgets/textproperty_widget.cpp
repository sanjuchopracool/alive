#include "textproperty_widget.h"
#include <QToolButton>
#include "exclusive_toolbutton_group.h"
#include "ui_textproperty_widget.h"
#include <gui/font_model.h>
#include <gui/utility/gui_utility.h>

namespace alive {
TextPropertyWidget::TextPropertyWidget(
    QWidget *parent)
    : QWidget(parent)
    , m_ui(new Ui::TextPropertyWidget)
{
    m_ui->setupUi(this);
    m_ui->sizeSpinBox->setMaximum(std::numeric_limits<double>::max());
    m_ui->strokeSpinBox->setMaximum(std::numeric_limits<double>::max());

    // update toolbuttons icon

    auto create_tool_button = [](const QString &icon) {
        QToolButton *btn = new QToolButton;
        btn->setCheckable(true);
        btn->setIconSize(QSize(24, 24));
        btn->setIcon(QIcon::fromTheme(icon));
        return btn;
    };

    m_butt_cap_button = create_tool_button("stroke-cap-butt");
    m_round_cap_button = create_tool_button("stroke-cap-round");
    m_square_cap_button = create_tool_button("stroke-cap-square");

    m_cap_group = new ExclusiveToolButtonGroup(Qt::Horizontal, this);
    m_cap_group->add_tool_button(m_butt_cap_button);
    m_cap_group->add_tool_button(m_round_cap_button);
    m_cap_group->add_tool_button(m_square_cap_button);

    m_bevel_join_button = create_tool_button("stroke-join-bevel");
    m_miter_join_button = create_tool_button("stroke-join-miter");
    m_round_join_button = create_tool_button("stroke-join-round");

    m_join_group = new ExclusiveToolButtonGroup(Qt::Horizontal, this);
    m_join_group->add_tool_button(m_bevel_join_button);
    m_join_group->add_tool_button(m_miter_join_button);
    m_join_group->add_tool_button(m_round_join_button);

    m_ui->capStyleLayout->addWidget(m_cap_group);
    m_ui->capStyleLayout->addStretch();

    m_ui->joinStyleLayout->addWidget(m_join_group);
    m_ui->joinStyleLayout->addStretch();

    // font model
    auto font_model = gui::FontModel::instance();
    m_ui->familyComboBox->setModel(font_model);

    auto update_style = [this, font_model]() {
        QSignalBlocker blk(m_ui->styleComboBox);
        m_ui->styleComboBox->clear();
        int index = 0;
        for (auto &str : font_model->styles()) {
            m_ui->styleComboBox->addItem(str, index);
            ++index;
        }
    };

    // select default
    if (font_model->rowCount(QModelIndex())) {
        font_model->set_current_index(0);
        m_ui->familyComboBox->setCurrentIndex(0);
    }

    update_style();
    // ui changed

    connect(m_ui->textEditor, &QTextEdit::textChanged, this, [this]() {
        m_text.m_text = m_ui->textEditor->toPlainText().toStdString();
        qDebug() << m_text.m_text;
        emit text_changed(m_text);
    });

    connect(m_ui->familyComboBox,
            &QComboBox::currentIndexChanged,
            this,
            [this, font_model, update_style]() {
                font_model->set_current_index(m_ui->familyComboBox->currentIndex());
                QString current_style_name = m_ui->styleComboBox->currentText();
                update_style();
                // try to find same style or use first style
                int style_index = m_ui->styleComboBox->findText(current_style_name);
                if (style_index == -1) {
                    style_index = 0;
                }

                if (m_text.m_font.family != m_ui->familyComboBox->currentIndex()
                    || m_text.m_font.style != style_index) {
                    m_text.m_font.family = m_ui->familyComboBox->currentIndex();
                    m_text.m_font.style = style_index;
                    emit text_changed(m_text);
                }
            });

    connect(m_ui->styleComboBox, &QComboBox::currentIndexChanged, this, [this]() {
        if (m_text.m_font.style != m_ui->styleComboBox->currentIndex()) {
            m_text.m_font.style = m_ui->styleComboBox->currentIndex();
            emit text_changed(m_text);
        }
    });

    connect(m_cap_group,
            &ExclusiveToolButtonGroup::tool_button_checked,
            this,
            [this](QToolButton *btn) {
                CapStyle style = CapStyle::e_Butt;
                if (btn == m_round_cap_button) {
                    style = CapStyle::e_Round;
                } else if (btn == m_square_cap_button) {
                    style = CapStyle::e_Square;
                }

                if (style != m_text.m_cap_style) {
                    m_text.m_cap_style = style;
                    emit text_changed(m_text);
                }
            });
    connect(m_join_group,
            &ExclusiveToolButtonGroup::tool_button_checked,
            this,
            [this](QToolButton *btn) {
                JoinStyle style = JoinStyle::e_Bevel;
                if (btn == m_miter_join_button) {
                    style = JoinStyle::e_Miter;
                } else if (btn == m_round_join_button) {
                    style = JoinStyle::e_Round;
                }

                if (style != m_text.m_join_style) {
                    m_text.m_join_style = style;
                    emit text_changed(m_text);
                }
            });

    connect(m_ui->sizeSpinBox, &QDoubleSpinBox::valueChanged, this, [this](double val) {
        m_text.m_font.size = val;
        emit text_changed(m_text);
    });
    connect(m_ui->strokeSpinBox, &QDoubleSpinBox::valueChanged, this, [this](double val) {
        m_text.m_stroke_width = val;
        emit text_changed(m_text);
    });

    connect(m_ui->fillCheckBox, &QCheckBox::checkStateChanged, this, [this]() {
        m_text.m_fill = m_ui->fillCheckBox->isChecked();
        emit text_changed(m_text);
    });

    connect(m_ui->strokeCheckBox, &QCheckBox::checkStateChanged, this, [this]() {
        m_text.m_stroke = m_ui->strokeCheckBox->isChecked();
        emit text_changed(m_text);
    });

    connect(m_ui->fillColorSelector,
            &alive::ColorLineEditor::color_changed,
            this,
            [this](const QColor &color) {
                m_text.m_fill_color = gui::qcolor_to_vec3d(color);
                emit text_changed(m_text);
            });

    connect(m_ui->strokeColorSelector,
            &alive::ColorLineEditor::color_changed,
            this,
            [this](const QColor &color) {
                m_text.m_stroke_color = gui::qcolor_to_vec3d(color);
                emit text_changed(m_text);
            });
}

TextPropertyWidget::~TextPropertyWidget()
{
    delete m_ui;
}

void TextPropertyWidget::update_from(
    const core::Text &text)
{
    if (m_text != text) {
        QSignalBlocker blk(this);
        m_text = text;
        update_ui();
    }
}

void TextPropertyWidget::reset()
{
    QSignalBlocker blk(this);
    m_text.m_text = {};
    update_ui();
}

void TextPropertyWidget::update_ui()
{
    m_ui->textEditor->setText(QString::fromStdString(m_text.m_text));

    // fill
    m_ui->fillCheckBox->setChecked(m_text.m_fill);
    m_ui->fillColorSelector->set_color(gui::qcolor_from_vec3d(m_text.m_fill_color));

    // stroke
    m_ui->strokeCheckBox->setChecked(m_text.m_stroke);
    m_ui->strokeSpinBox->setValue(m_text.m_stroke_width);
    m_ui->strokeColorSelector->set_color(gui::qcolor_from_vec3d(m_text.m_stroke_color));

    // font
    m_ui->sizeSpinBox->setValue(m_text.m_font.size);

    // cap
    switch (m_text.m_cap_style) {
    case alive::CapStyle::e_Butt:
        m_cap_group->select(m_butt_cap_button, true);
        break;
    case alive::CapStyle::e_Round:
        m_cap_group->select(m_round_cap_button, true);
        break;
    case alive::CapStyle::e_Square:
        m_cap_group->select(m_square_cap_button, true);
        break;
    default:
        break;
    }

    // join
    switch (m_text.m_join_style) {
    case alive::JoinStyle::e_Bevel:
        m_join_group->select(m_bevel_join_button, true);
        break;
    case alive::JoinStyle::e_Miter:
        m_join_group->select(m_miter_join_button, true);
        break;
    case alive::JoinStyle::e_Round:
        m_join_group->select(m_round_join_button, true);
        break;
    default:
        break;
    }

    // font
    m_ui->familyComboBox->setCurrentIndex(m_text.m_font.family);
    m_ui->styleComboBox->setCurrentIndex(m_text.m_font.style);
}
} // namespace alive
