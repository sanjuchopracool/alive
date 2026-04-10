#include "tree_column_delegate.h"
#include "delegate_utility.h"
#include <gui/document/composition_document.h>
#include <gui/framework/theme/color_palette.h>
#include <gui/icon_manager.h>
#include <gui/quick/range_selector_helper.h>
#include <gui/theme/layer_color_model.h>
#include <gui/theme/timeline_style.h>
#include <QEvent>
#include <QLineEdit>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>

namespace alive {

TreeColumnDelegate::TreeColumnDelegate(QAbstractItemModel *model, QObject *parent)
    : QAbstractItemDelegate{parent}
    , m_model(model)
{
    m_height = alive::theme::TimelineSize::instance()->item_height();
}

QSize TreeColumnDelegate::sizeHint(const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
    return QSize(100, m_height);
}

void TreeColumnDelegate::paint(QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    const auto *sizes = alive::theme::TimelineSize::instance();
    const auto *colors = alive::theme::ColorPalette::instance();
    const auto *icon_mgr = alive::IconManager::instance();

    painter->save();
    using namespace alive::model;
    bool is_layer = m_model->data(index, CompositionDocument::IsLayer).toBool();
    const int spacing = sizes->item_spacing();
    QRect rect = option.rect;
    if (is_layer) {
        int icon_height = sizes->icon_size().height();
        QRect icon_rect(rect.left() + spacing,
                        rect.top() + (m_height - icon_height) / 2,
                        sizes->icon_size().width(),
                        icon_height);
        QColor color = m_model->data(index, CompositionDocument::LayerColor).value<QColor>();
        painter->fillRect(icon_rect, color);

        rect.setLeft(rect.left() + rect.width() - m_layer_column_width);
        const QString &text = m_model->data(index, Qt::DisplayRole).toString();
        if (option.state & QStyle::QStyle::State_Selected) {
            painter->fillRect(rect, colors->accent_color());
            painter->setPen(colors->primary_background_color());
        }
        rect.setLeft(rect.left() + spacing);
        painter->drawText(rect, Qt::AlignLeft, text);
    } else {
        // draw animating icon
        bool can_animate = m_model->data(index, CompositionDocument::CanAnimate).toBool();
        if (can_animate) {
            int icon_height = sizes->icon_size().height();
            const int spacing = sizes->item_spacing();
            QRect icon_rect(rect.left() + spacing,
                            rect.top() + (m_height - icon_height) / 2,
                            sizes->icon_size().width(),
                            icon_height);
            const bool is_animating
                = m_model->data(index, CompositionDocument::IsAnimating).toBool();
            if (is_animating) {
                icon_mgr->get_icon(IconManager::e_AnimationEnabled).paint(painter, icon_rect);
            } else {
                icon_mgr->get_icon(IconManager::e_AnimationDisabled).paint(painter, icon_rect);
            }
            rect.setLeft(icon_rect.right() + spacing);
        }

        const QString &text = m_model->data(index, Qt::DisplayRole).toString();
        if (option.state & QStyle::QStyle::State_Selected) {
            painter->fillRect(rect, colors->accent_color());
            painter->setPen(colors->primary_background_color());
        }
        painter->drawText(rect.adjusted(sizes->item_spacing(), 0, 0, 0), Qt::AlignLeft, text);
    }
    painter->restore();
}

void TreeColumnDelegate::set_layer_column_widht(int width)
{
    m_layer_column_width = width;
}

QWidget *TreeColumnDelegate::createEditor(QWidget *parent,
                                          const QStyleOptionViewItem &option,
                                          const QModelIndex &index) const
{
    m_editor = new QLineEdit(parent);
    m_editor->installEventFilter(const_cast<TreeColumnDelegate *>(this));
    return m_editor;
}

void TreeColumnDelegate::updateEditorGeometry(QWidget *editor,
                                              const QStyleOptionViewItem &option,
                                              const QModelIndex &index) const
{
    const auto *sizes = alive::theme::TimelineSize::instance();
    bool is_layer = index.data(alive::model::CompositionDocument::IsLayer).toBool();
    QRect rect = option.rect;
    if (is_layer) {
        rect.setLeft(rect.left() + 2 * sizes->item_spacing() + sizes->icon_size().width());
    }
    editor->setGeometry(rect);
}

void TreeColumnDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    qobject_cast<QLineEdit *>(editor)->setText(index.data().toString());
}

void TreeColumnDelegate::setModelData(QWidget *editor,
                                      QAbstractItemModel *model,
                                      const QModelIndex &index) const
{
    if (editor == m_editor) {
        model->setData(index, m_editor->text(), alive::model::CompositionDocument::SetName);
    }
}

bool TreeColumnDelegate::eventFilter(QObject *watched, QEvent *event)
{
    if ((m_editor == watched) && event->type() == QEvent::KeyPress) {
        QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
        int key = key_event->key();
        if (key == Qt::Key_Escape) {
            emit closeEditor(m_editor, QAbstractItemDelegate::NoHint);
            m_editor = nullptr;
            return false;
        } else if (key == Qt::Key_Return || key == Qt::Key_Enter) {
            emit commitData(m_editor); //save changes
            emit closeEditor(m_editor, QAbstractItemDelegate::NoHint);
            m_editor = nullptr;
            return true;
        }
    }
    return false;
}

bool TreeColumnDelegate::editorEvent(QEvent *event,
                                     QAbstractItemModel *model,
                                     const QStyleOptionViewItem &option,
                                     const QModelIndex &index)
{
    QMouseEvent *me = nullptr;
    if (event->type() == QEvent::MouseButtonPress) {
        me = static_cast<QMouseEvent *>(event);
    }
    if (me) {
        using namespace alive::model;
        if (me->button() == Qt::LeftButton) {
            const QRect &rect = option.rect;
            ClickPosition icon_pos = left_icon_position_for_click(me->position(), option.rect);
            if (icon_pos == ClickPosition::e_First) {
                bool is_layer = m_model->data(index, CompositionDocument::IsLayer).toBool();
                if (is_layer) {
                    // open menu here
                    QMenu menu;
                    int selected
                        = m_model->data(index, CompositionDocument::LayerColorIndex).toInt();
                    const auto *sizes = alive::theme::TimelineSize::instance();
                    QPixmap icon(sizes->icon_size());
                    QAction *def = nullptr;
                    auto layer_colors = alive::gui::LayerColorModel::instance();
                    for (int i = 0; i < layer_colors->count(); ++i) {
                        if (i == layer_colors->unset_index())
                            continue;

                        auto color = layer_colors->editor_color_at_index(i);
                        QAction *act = new QAction(color.name, &menu);
                        icon.fill(color.color);
                        act->setIcon(icon);
                        act->setData(i);
                        if (i == selected) {
                            def = act;
                        }
                        menu.addAction(act);
                    }
                    QAction *act = menu.exec(me->globalPosition().toPoint(), def);
                    if (act) {
                        int new_index = act->data().toInt();
                        if (new_index != selected) {
                            m_model->setData(index, new_index, CompositionDocument::LayerColorIndex);
                            return true;
                        }
                    }

                } else if (m_model->data(index, CompositionDocument::CanAnimate).toBool()) {
                    int role = m_model->data(index, CompositionDocument::IsAnimating).toBool()
                                   ? CompositionDocument::RemoveAllKeyFrame
                                   : CompositionDocument::AddKeyFrame;
                    m_model->setData(index, true, role);
                    return true;
                }
            }
        }
    }
    return QAbstractItemDelegate::editorEvent(event, model, option, index);
}
} // namespace alive
