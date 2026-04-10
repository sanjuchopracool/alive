#include "feature_column_delegate.h"
#include "delegate_utility.h"
#include <gui/document/composition_document.h>
#include <gui/icon_manager.h>
#include <gui/quick/range_selector_helper.h>
#include <gui/framework/theme/color_palette.h>
#include <gui/theme/timeline_style.h>
#include <gui/timeline_model_helpers.h>
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <core/profiler.h>

namespace alive {

FeatureColumnDelegate::FeatureColumnDelegate(QAbstractItemModel *model, QObject *parent)
    : QAbstractItemDelegate{parent}
    , m_model(model)
{
    m_height = alive::theme::TimelineSize::instance()->item_height();
}

QSize FeatureColumnDelegate::sizeHint(const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const
{
    return QSize(100, m_height);
}

void FeatureColumnDelegate::paint(QPainter *painter,
                                  const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const
{
    // AutoProfiler p("Paint event");
    const auto *colors = alive::theme::ColorPalette::instance();
    const auto *sizes = alive::theme::TimelineSize::instance();

    painter->save();
    painter->setPen(colors->stroke_color());
    using namespace alive::model;
    bool is_object = m_model->data(index, CompositionDocument::IsObject).toBool();
    const int spacing = sizes->item_spacing();
    const QRect &rect = option.rect;
    const auto *icon_mgr = IconManager::instance();
    int icon_height = sizes->icon_size().height();
    QRect icon_rect(rect.left() + spacing,
                    rect.top() + (m_height - icon_height) / 2,
                    sizes->icon_size().width(),
                    icon_height);
    if (is_object) {
        if (m_model->data(index, CompositionDocument::ObjectSupportVisibility).toBool()) {
            painter->fillRect(icon_rect, colors->primary_background_color());

            if (m_model->data(index, CompositionDocument::ObjectVisible).toBool()) {
                icon_mgr->get_icon(IconManager::e_Visible).paint(painter, icon_rect);
            }
        }

        // lock state
        icon_rect.moveLeft(icon_rect.right() + spacing);
        if (m_model->data(index, CompositionDocument::IsLayer).toBool()) {
            painter->fillRect(icon_rect, colors->primary_background_color());
            if (m_model->data(index, CompositionDocument::LayerLocked).toBool()) {
                icon_mgr->get_icon(IconManager::e_Lock).paint(painter, icon_rect);
            }
        }
    } else {
        // a property
        if (m_model->data(index, CompositionDocument::IsAnimating).toBool()) {
            KeyFrameStatusValue keyframe_status = m_model
                                                      ->data(index,
                                                             CompositionDocument::KeyFrameStatus)
                                                      .value<KeyFrameStatusValue>();
            const auto *icon_mgr = alive::IconManager::instance();
            if (keyframe_status.on_left) {
                if (keyframe_status.left_scripted) {
                    icon_mgr->get_icon(IconManager::e_ScriptedKeyframeAtLeft)
                        .paint(painter, icon_rect);
                } else {
                    icon_mgr->get_icon(IconManager::e_KeyframeAtLeft).paint(painter, icon_rect);
                }
            } else {
                icon_mgr->get_icon(IconManager::e_KeyframeNotAtLeft).paint(painter, icon_rect);
            }

            icon_rect.moveLeft(icon_rect.right() + spacing);
            if (keyframe_status.at_keyframe) {
                if (keyframe_status.scripted) {
                    icon_mgr->get_icon(IconManager::e_AtScriptedKeyFrame).paint(painter, icon_rect);
                } else {
                    icon_mgr->get_icon(IconManager::e_AtKeyFrame).paint(painter, icon_rect);
                }
            } else {
                icon_mgr->get_icon(IconManager::e_NotAtKeyFrame).paint(painter, icon_rect);
            }

            icon_rect.moveLeft(icon_rect.right() + spacing);
            if (keyframe_status.on_right) {
                if (keyframe_status.right_scripted) {
                    icon_mgr->get_icon(IconManager::e_ScriptedKeyframeAtRight)
                        .paint(painter, icon_rect);
                } else {
                    icon_mgr->get_icon(IconManager::e_KeyframeAtRight).paint(painter, icon_rect);
                }
            } else {
                icon_mgr->get_icon(IconManager::e_KeyframeNotAtRight).paint(painter, icon_rect);
            }
        }
    }
    painter->restore();
}

bool FeatureColumnDelegate::editorEvent(QEvent *event,
                                        QAbstractItemModel *model,
                                        const QStyleOptionViewItem &option,
                                        const QModelIndex &index)
{
    QMouseEvent *me = nullptr;
    if (event->type() == QEvent::MouseButtonRelease) {
        me = static_cast<QMouseEvent *>(event);
    }
    if (me && me->button() == Qt::LeftButton) {
        AutoProfiler p("HAndling event");
        const auto *sizes = alive::theme::TimelineSize::instance();
        using namespace alive::model;
        bool is_object = m_model->data(index, CompositionDocument::IsObject).toBool();
        const int spacing = sizes->item_spacing();
        const QRect &rect = option.rect;
        if (is_object) {
            int icon_height = sizes->icon_size().height();
            QRect icon_rect(rect.left() + spacing,
                            rect.top() + (m_height - icon_height) / 2,
                            sizes->icon_size().width(),
                            icon_height);

            // visible
            if (icon_rect.contains(me->pos())) {
                if (m_model->data(index, CompositionDocument::ObjectSupportVisibility).toBool()) {
                    m_model->setData(index,
                                     !m_model->data(index, CompositionDocument::ObjectVisible)
                                          .toBool(),
                                     CompositionDocument::ObjectVisible);
                    return true;
                }
            }

            // lock state
            icon_rect.moveLeft(icon_rect.right() + spacing);
            if (icon_rect.contains(me->pos())) {
                if (m_model->data(index, CompositionDocument::IsLayer).toBool()) {
                    if (icon_rect.contains(me->pos())) {
                        m_model->setData(index,
                                         !m_model->data(index, CompositionDocument::LayerLocked)
                                              .toBool(),
                                         CompositionDocument::LayerLocked);
                        return true;
                    }
                }
            }
        } else {
            // a property
            if (m_model->data(index, CompositionDocument::IsAnimating).toBool()) {
                ClickPosition icon_pos = left_icon_position_for_click(me->position(), option.rect);
                if (icon_pos != ClickPosition::e_None) {
                    KeyFrameStatusValue keyframe_status
                        = m_model->data(index, CompositionDocument::KeyFrameStatus)
                              .value<KeyFrameStatusValue>();

                    if (icon_pos == ClickPosition::e_First) {
                        if (keyframe_status.on_left) {
                            emit go_to_time(keyframe_status.previous);
                        }
                    } else if (icon_pos == ClickPosition::e_Second) {
                        int role = keyframe_status.at_keyframe ? CompositionDocument::RemoveKeyFrame
                                                               : CompositionDocument::AddKeyFrame;
                        m_model->setData(index, keyframe_status.at_keyframe, role);
                    } else if (icon_pos == ClickPosition::e_Third) {
                        if (keyframe_status.on_right) {
                            emit go_to_time(keyframe_status.next);
                        }
                    }
                    return true;
                }
            }
        }
    }
    return QAbstractItemDelegate::editorEvent(event, model, option, index);
}

} // namespace alive
