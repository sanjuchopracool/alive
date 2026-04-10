#include "font_setting_widget.h"
#include "ui_font_setting_widget.h"
#include <gui/font_model.h>

namespace alive {
FontSettingWidget::FontSettingWidget(QWidget *parent)
    : QWidget(parent)
    , m_ui(new Ui::FontSettingWidget)
{
    m_ui->setupUi(this);
    m_ui->sb_size->setMinimum(0.0);
    m_ui->sb_size->setMaximum(std::numeric_limits<double>::max());

    // font model
    auto font_model = gui::FontModel::instance();
    m_ui->cb_family->setModel(font_model);

    auto update_style = [this, font_model]() {
        QSignalBlocker blk(m_ui->cb_style);
        m_ui->cb_style->clear();
        int index = 0;
        for (auto &str : font_model->styles()) {
            m_ui->cb_style->addItem(str, index);
            ++index;
        }
    };

    // select default
    if (font_model->rowCount(QModelIndex())) {
        font_model->set_current_index(0);
        m_ui->cb_family->setCurrentIndex(0);
    }

    connect(m_ui->cb_family,
            &QComboBox::currentIndexChanged,
            this,
            [this, font_model, update_style]() {
                font_model->set_current_index(m_ui->cb_family->currentIndex());
                QString current_style_name = m_ui->cb_style->currentText();
                update_style();
                // try to find same style or use first style
                if (!signalsBlocked()) {
                    int style_index = m_ui->cb_style->findText(current_style_name);
                    if (style_index == -1) {
                        style_index = 0;
                    }

                    if (m_font.family != m_ui->cb_family->currentIndex()
                        || m_font.style != style_index) {
                        m_font.family = m_ui->cb_family->currentIndex();
                        m_font.style = style_index;
                        emit font_changed(m_font);
                    }
                }
            });

    connect(m_ui->cb_style, &QComboBox::currentIndexChanged, this, [this]() {
        if (m_font.style != m_ui->cb_style->currentIndex()) {
            m_font.style = m_ui->cb_style->currentIndex();
            emit font_changed(m_font);
        }
    });

    connect(m_ui->sb_size, &QDoubleSpinBox::valueChanged, this, [this](double val) {
        m_font.size = val;
        emit font_changed(m_font);
    });
    update_style();
}

FontSettingWidget::~FontSettingWidget()
{
    delete m_ui;
}

void FontSettingWidget::set_font(const core::Font &font)
{
    if (m_font != font) {
        QSignalBlocker blk(this);
        m_font = font;
        update_ui();
    }
}

void FontSettingWidget::update_ui()
{
    m_ui->sb_size->setValue(m_font.size);
    m_ui->cb_family->setCurrentIndex(m_font.family);
    m_ui->cb_style->setCurrentIndex(m_font.style);
}
} // namespace alive
