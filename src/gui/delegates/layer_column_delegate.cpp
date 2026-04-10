#include "layer_column_delegate.h"
#include <gui/document/composition_document.h>
#include <gui/quick/range_selector_helper.h>
#include <gui/framework/theme/color_palette.h>
#include <gui/theme/timeline_style.h>
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>

namespace alive {

LayerColumnDelegate::LayerColumnDelegate(QAbstractItemModel *model, QObject *parent)
    : QAbstractItemDelegate{parent}
    , m_model(model)
{
    m_height = alive::theme::TimelineSize::instance()->item_height();
}

QSize LayerColumnDelegate::sizeHint(const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const
{
    return QSize(100, m_height);
}

void LayerColumnDelegate::paint(QPainter *painter,
                                const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    painter->save();
    using namespace alive::model;
    bool is_layer = m_model->data(index, CompositionDocument::IsLayer).toBool();
    const QRect &rect = option.rect;
    if (is_layer) {
        const QRect &rect(option.rect);
        const QString &text = m_model->data(index, Qt::DisplayRole).toString();
        const auto *colors = alive::theme::ColorPalette::instance();

        if (option.state & QStyle::QStyle::State_Selected) {
            painter->fillRect(rect, colors->accent_color());
            painter->setPen(colors->primary_background_color());
        }

        painter->drawText(option.rect.adjusted(0, 0, 200, 0), Qt::AlignLeft, text);
    }
    painter->restore();
}

bool LayerColumnDelegate::editorEvent(QEvent *event,
                                      QAbstractItemModel *model,
                                      const QStyleOptionViewItem &option,
                                      const QModelIndex &index)
{
    qDebug() << Q_FUNC_INFO;
    return QAbstractItemDelegate::editorEvent(event, model, option, index);
}

} // namespace alive
