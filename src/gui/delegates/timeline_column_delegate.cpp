#include "timeline_column_delegate.h"
#include <gui/document/composition_document.h>
#include <gui/framework/theme/color_palette.h>
#include <gui/quick/range_selector_helper.h>
#include <gui/theme/layer_color_model.h>
#include <gui/theme/timeline_style.h>
#include <gui/timeline_model_helpers.h>
#include <gui/utility/keyframe_gui_utility.h>
#include <QEvent>
#include <QPainter>

namespace alive {

TimelineColumnDelegate::TimelineColumnDelegate(QAbstractItemModel *model,
                                               const alive::gui::TimelineHelper *helper,
                                               QObject *parent)
    : QAbstractItemDelegate{parent}
    , m_model(model)
    , m_helper(helper)
{
    m_height = alive::theme::TimelineSize::instance()->item_height();
}

QSize TimelineColumnDelegate::sizeHint(const QStyleOptionViewItem &option,
                                       const QModelIndex &index) const
{
    return QSize(100, m_height);
}

void TimelineColumnDelegate::paint(QPainter *painter,
                                   const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
    using namespace alive::model;
    const auto *colors = alive::theme::ColorPalette::instance();
    painter->save();
    painter->setPen(colors->stroke_color());
    QRectF rectf(option.rect);
    using namespace alive::model;
    bool is_layer = m_model->data(index, CompositionDocument::IsLayer).toBool();
    painter->setClipRect(rectf);
    if (is_layer) {
        // draw layer in and out
        const LayerInOutPoints in_out_value
            = m_model->data(index, CompositionDocument::LayerInOutRange).value<LayerInOutPoints>();

        QColor color = m_model->data(index, CompositionDocument::LayerColor).value<QColor>();
        if (!(option.state & QStyle::State_Selected)) {
            color.setAlpha(128);
        }
        QRectF fill_rect(rectf);
        qreal initial_left = fill_rect.left();
        fill_rect.setLeft(initial_left + m_helper->position_for_frame(in_out_value.in_point));
        fill_rect.setRight(initial_left + m_helper->position_for_frame(in_out_value.out_point));
        painter->fillRect(fill_rect, color);
    } else {
        bool is_animating = m_model->data(index, CompositionDocument::IsAnimating).toBool();
        if (is_animating) {
            Property *property = nullptr;
            alive::TreeItem *item
                = m_model->data(index, CompositionDocument::ItemRole).value<alive::TreeItem *>();
            if (item) {
                property = item->to_property();
            }

            // draw key frames
            if (property) {
                qreal y = option.rect.center().y();
                qreal x = option.rect.left();
                painter->setRenderHint(QPainter::Antialiasing);
                for (const auto *keyframe : property->get_keyframes()) {
                    qreal frame_x = x + m_helper->position_for_frame(keyframe->time());
                    painter->save();
                    painter->setPen(colors->stroke_color());
                    painter->translate(frame_x, y);
                    bool scripted = keyframe->expression_enabled()
                                    && !keyframe->expression().empty();
                    if (keyframe->is_selected_in_editor()) {
                        if (scripted) {
                            painter->setBrush(colors->play_cursor_color());
                        } else {
                            painter->setBrush(colors->link_color());
                        }
                    } else {
                        if (scripted) {
                            painter->setBrush(colors->play_cursor_color().lighter());
                        } else {
                            painter->setBrush(colors->primary_font_color());
                        }
                    }
                    painter->drawPath(alive::keyFrameGuiUtility::path_for_keyframe(keyframe));
                    painter->restore();
                }
            }
        }
    }
    painter->restore();
}

bool TimelineColumnDelegate::editorEvent(QEvent *event,
                                         QAbstractItemModel *model,
                                         const QStyleOptionViewItem &option,
                                         const QModelIndex &index)
{
    if (event->type() == QEvent::MouseMove) {
        return true;
    }

    return false;
}

} // namespace alive
