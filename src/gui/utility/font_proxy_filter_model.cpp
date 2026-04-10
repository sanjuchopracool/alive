#include "font_proxy_filter_model.h"
#include <gui/font_model.h>

namespace alive {

FontProxyFilterModel *the_instance = nullptr;

FontProxyFilterModel::FontProxyFilterModel(gui::FontModel *source)
    : QSortFilterProxyModel{source}
    , m_source(source)
{
    setSourceModel(m_source);
    connect(m_source, &gui::FontModel::current_index_changed, this, [this]() {
        set_current_index(mapFromSource(sourceModel()->index(m_source->current_index(), 0)).row());
    });

    connect(m_source, &gui::FontModel::styles_changed, this, [this]() {
        update_styles_from_source();
    });

    connect(m_source, &gui::FontModel::style_index_changed, this, [this]() {
        update_styles_index_from_source();
    });
}

void FontProxyFilterModel::update_styles_from_source()
{
    m_styles.clear();
    m_parent_styles_index.clear();
    int index = 0;
    int famliy_id = m_source->current_index();
    for (const auto &style : m_source->styles()) {
        bool append = true;
        if (!m_show_system_fonts && m_source->is_system_font(famliy_id, index)) {
            append = false;
        }

        if (append) {
            m_styles.emplaceBack(style);
            m_parent_styles_index.emplaceBack(index);
        }
        index++;
    }
    emit styles_changed();
}

void FontProxyFilterModel::update_styles_index_from_source()
{
    int source_style_index = m_source->style_index();
    int index = 0;
    for (int parent : m_parent_styles_index) {
        if (parent == source_style_index) {
            set_style_index(index);
            return;
        }
        index++;
    }
}

bool FontProxyFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (!m_show_system_fonts) {
        auto index = sourceModel()->index(source_row, 0, source_parent);
        bool has_db_font = m_source->data(index, gui::FontModel::e_DBFont).toBool();
        if (has_db_font) {
            return false;
        }
    }

    return true;
}

FontProxyFilterModel *FontProxyFilterModel::instance()
{
    return the_instance;
}

void FontProxyFilterModel::init(gui::FontModel *source)
{
    if (!the_instance) {
        the_instance = new FontProxyFilterModel(source);
    }
}

int FontProxyFilterModel::current_index() const
{
    return m_index;
}

void FontProxyFilterModel::set_current_index(int new_index)
{
    if (m_index == new_index)
        return;

    m_index = new_index;
    m_source->set_current_index(mapToSource(index(new_index, 0)).row());
    emit current_index_changed();
}

const QStringList &FontProxyFilterModel::styles() const
{
    return m_styles;
}

int FontProxyFilterModel::style_index() const
{
    return m_style_index;
}

void FontProxyFilterModel::set_style_index(int new_style_index)
{
    if (m_style_index >= m_styles.size())
        return;

    if (m_style_index == new_style_index)
        return;

    m_style_index = new_style_index;
    m_source->set_style_index(m_parent_styles_index[m_style_index]);
    emit style_index_changed();
}
} // namespace alive
