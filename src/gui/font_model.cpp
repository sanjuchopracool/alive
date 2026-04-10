#include "font_model.h"
#include <core/asset_management/font_manager.h>
#include <QFontDatabase>
#include <QRegularExpression>

namespace alive::gui {
QRegularExpression k_reg_ex("\\[(.*)\\]");

FontModel::FontModel(QObject *parent)
    : QAbstractListModel(parent)
{
    if (m_index == -1) {
        m_index = 0;
    }
    update_styles();
}

void FontModel::update_styles()
{
    const auto &font_manager = core::FontManager::instance();
    if (m_index >= 0 && m_index < font_manager.families_count()) {
        m_styles.clear();
        auto style_count = font_manager.style_count(m_index);
        for (size_t i = 0; i < style_count; ++i) {
            m_styles.emplace_back(font_manager.style_name(m_index, i).c_str());
        }

        emit styles_changed();
        if (m_style_index >= m_styles.size()) {
            int new_index = m_styles.indexOf("Regular");
            if (new_index == -1 && m_styles.size()) {
                new_index = 0;
            }
            m_style_index = new_index;
        }

        emit style_index_changed();
    }
}

int FontModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return core::FontManager::instance().families_count();
}

QVariant FontModel::data(const QModelIndex &index, int role) const
{
    const auto &font_manager = core::FontManager::instance();
    if (index.row() < font_manager.families_count()) {
        switch (role) {
        case Qt::DisplayRole:
            return font_manager.family_name(index.row()).c_str();
        case e_SystemFont:
            return font_manager.family_contains_system_fonts(index.row());
        case e_DBFont:
            return font_manager.family_contains_db_fonts(index.row());
        default:
            break;
        }
    }
    return {};
}

int FontModel::font_famliy_index(QString family) const
{
    return core::FontManager::instance().family_index(family.toStdString());
}

QString FontModel::family_at_index(int index) const
{
    const auto &font_manager = core::FontManager::instance();
    if (index >= 0 && index < font_manager.families_count()) {
        return QString::fromStdString(core::FontManager::instance().family_name(index));
    }
    return {};
}

QString FontModel::style_at_index(int index) const
{
    if (index >= 0 && index < m_styles.size()) {
        return m_styles[index];
    }
    return {};
}

bool FontModel::is_system_font(int index, int style_index) const
{
    const auto &font_manager = core::FontManager::instance();
    return font_manager.is_system(index, style_index);
}

int FontModel::current_index() const
{
    return m_index;
}

void FontModel::set_current_index(int new_index)
{
    if (m_index == new_index)
        return;
    m_index = new_index;

    update_styles();
    emit current_index_changed();
}

void FontModel::set_current_family_and_style(int new_family, int new_style)
{
    if (m_style_index == new_style && m_index == new_family)
        return;

    m_style_index = new_style;
    set_current_index(new_family);
}

const QStringList &FontModel::styles() const
{
    return m_styles;
}

int FontModel::style_index() const
{
    return m_style_index;
}

void FontModel::set_style_index(int new_style_index)
{
    if (m_style_index == new_style_index)
        return;
    m_style_index = new_style_index;
    emit style_index_changed();
}

} // namespace alive::gui
