#include "edit_column_delegate.h"
#include "delegate_utility.h"
#include <gui/cursor_manager.h>
#include <gui/document/composition_document.h>
#include <gui/framework/theme/color_palette.h>
#include <gui/icon_manager.h>
#include <gui/model/property/gui_property.h>
#include <gui/model/property/property_editor_callbacks.h>
#include <gui/theme/timeline_style.h>
#include <gui/utility/gui_utility.h>
#include <gui/widgets/utility.h>
#include <QApplication>
#include <QColorDialog>
#include <QComboBox>
#include <QEvent>
#include <QFontMetrics>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>

namespace alive {

namespace {
constexpr int k_prop_editor_width = 50;
void draw_color(const QColor &color,
                QPainter *painter,
                const QStyleOptionViewItem &option,
                const alive::theme::ColorPalette *colors,
                const alive::theme::TimelineSize *sizes)
{
    const auto &icon_size = sizes->icon_size();
    QRect icon_rect(0, 0, icon_size.width(), icon_size.height());
    QPoint new_center(option.rect.left() + icon_rect.width() / 2 + sizes->item_spacing(),
                      option.rect.center().y());
    icon_rect.moveCenter(new_center);
    painter->setPen(colors->stroke_color());
    painter->setBrush(color);
    painter->drawRect(icon_rect);
}

void draw_shape_content(QPainter *painter,
                        const QStyleOptionViewItem &option,
                        const alive::theme::ColorPalette *colors,
                        const alive::theme::TimelineSize *sizes)
{
    const auto &icon_size = sizes->icon_size();

    QRect icon_rect(0, 0, icon_size.width(), icon_size.height());
    QPoint new_center(option.rect.right() - icon_rect.width() / 2 - sizes->item_spacing(),
                      option.rect.center().y());
    icon_rect.moveCenter(new_center);

    const auto *icon_mgr = IconManager::instance();
    icon_mgr->get_icon(IconManager::e_GroupAdd).paint(painter, icon_rect);

    QRectF rect(option.rect);
    QFont font = QApplication::font();
    QFontMetrics fmc(font);
    rect.setHeight(fmc.height());
    rect.moveRight(icon_rect.left() - sizes->item_spacing());
    rect.moveCenter(QPointF(rect.center().x(), option.rect.center().y()));
    painter->drawText(rect, Qt::AlignRight, QObject::tr("Add:"));
}

enum class EditingContext { e_None, e_Hover, e_DragStarted, e_Dragging, e_Editing };
} // namespace

struct BaseEditingState;
struct EditColumnDelegatePrivateData
{
    EditColumnDelegatePrivateData(const alive::model::CompositionDocument *doc,
                                  QAbstractItemModel *model,
                                  EditColumnDelegate *delegate)
        : document(doc)
        , m_model(model)
        , object(delegate)
    {}
    ~EditColumnDelegatePrivateData() {}

    const alive::model::CompositionDocument *document;
    QAbstractItemModel *m_model;
    EditColumnDelegate *object = nullptr;
    int m_height = 10;
    bool m_is_handling_timeline = false;
    // specific to hover
    QModelIndex current_index;
    EditingField current_field = alive::EditingField::e_None;
    EditingContext editing_context = EditingContext::e_None;
    QPointF drag_start_pos;
    int m_cursor = -1;
    std::unique_ptr<BaseEditingState> current_state;

    void reset_from_edit_state(QWidget *editor);
    void drag(QPointF p)
    {
        if (editing_context != EditingContext::e_Dragging)
            return;

        using namespace alive::model;
        QPointF delta = p - drag_start_pos;
        qreal delta_val = delta.x() - delta.y();
        int role = current_field == alive::EditingField::e_First
                       ? CompositionDocument::ChangeValueT1
                       : CompositionDocument::ChangeValueT2;
        m_model->setData(current_index, delta_val, role);
        drag_start_pos = p;
    }
    void restore_cursor()
    {
        if (m_cursor != -1) {
            alive::gui::CursorManager::instance()->restore_cursor(m_cursor);
            m_cursor = -1;
        }
    }

    EditingField field_from_position(const QVariant &var_value,
                                     const QModelIndex &index,
                                     const QPointF &p,
                                     const QStyleOptionViewItem &option,
                                     const alive::theme::TimelineSize *sizes) const
    {
        alive::gui::PropertyTextValue prop_text = var_value.value<alive::gui::PropertyTextValue>();
        QFont font = QApplication::font();
        QFontMetrics fmc(font);
        qreal y = p.y() - option.rect.center().y();
        // check vertical range first
        if (std::abs(y) <= fmc.height() / 2.0) {
            // skip left spacing
            auto spacing = sizes->item_spacing();
            if (index.data(model::CompositionDocument::IsLinkable).toBool()) {
                spacing += sizes->icon_size().width() + sizes->item_spacing();
            }
            qreal x = p.x() - option.rect.left() - spacing;
            if (x > 0) {
                int w = fmc.horizontalAdvance(prop_text.t1);
                // check t1
                if (x <= w) {
                    return alive::EditingField::e_First;
                }
                if (prop_text.t2.length()) {
                    x -= w;
                    x -= fmc.horizontalAdvance(prop_text.seperator);

                    w = fmc.horizontalAdvance(prop_text.t2);
                    if (x <= w) {
                        return alive::EditingField::e_Second;
                    }
                }
            }
        }
        return alive::EditingField::e_None;
    }
};

struct BaseEditingState
{
    virtual bool mouse_press(QMouseEvent *ev,
                             QAbstractItemModel *model,
                             const QStyleOptionViewItem &option,
                             const QModelIndex &index)
    {
        return false;
    }

    virtual bool mouse_release(QMouseEvent *ev,
                               QAbstractItemModel *model,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index)
    {
        return false;
    }

    virtual bool mouse_move(QMouseEvent *ev,
                            QAbstractItemModel *model,
                            const QStyleOptionViewItem &option,
                            const QModelIndex &index)
    {
        return false;
    }

protected:
    BaseEditingState(EditColumnDelegatePrivateData *data)
        : m_d(data)
    {}

    EditColumnDelegatePrivateData *m_d;
};

struct DragState : public BaseEditingState
{
    DragState(EditColumnDelegatePrivateData *data)
        : BaseEditingState(data)
    {}

    bool mouse_move(QMouseEvent *ev,
                    QAbstractItemModel *model,
                    const QStyleOptionViewItem &option,
                    const QModelIndex &index) override
    {
        if (m_d->m_is_handling_timeline) {
            return false;
        }
        if (m_d->editing_context == EditingContext::e_DragStarted
            || m_d->editing_context == EditingContext::e_Dragging) {
            m_d->editing_context = EditingContext::e_Dragging;
            m_d->drag(ev->position());
            return true;
        }
        return true;
    }
    bool mouse_release(QMouseEvent *ev,
                       QAbstractItemModel *model,
                       const QStyleOptionViewItem &option,
                       const QModelIndex &index) override;
};

struct EditState : public BaseEditingState
{
    EditState(EditColumnDelegatePrivateData *data, const QModelIndex &index)
        : BaseEditingState(data)
    {
        m_d->restore_cursor();
        m_d->object->request_editor(index);
    }
};

struct NormalState : public BaseEditingState
{
    NormalState(EditColumnDelegatePrivateData *data)
        : BaseEditingState(data)
    {}

    bool mouse_press(QMouseEvent *ev,
                     QAbstractItemModel *model,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) override
    {
        if (ev->button() != Qt::LeftButton) {
            return false;
        }

        if (m_d->editing_context == EditingContext::e_Hover && m_d->current_field != alive::EditingField::e_None) {
            m_d->drag_start_pos = ev->position();
            m_d->editing_context = EditingContext::e_DragStarted;
            // change current state
            m_d->current_state.reset(std::make_unique<DragState>(m_d).release());
            return true;
        }

        using namespace alive::gui;
        EditColumnValue value
            = model->data(index, CompositionDocument::EditValue).value<EditColumnValue>();
        ClickPosition left_click_pos = left_icon_position_for_click(ev->position(), option.rect);
        ClickPosition right_click_pos = right_icon_position_for_click(ev->position(), option.rect);
        if (value.m_type == EditColumnValue::ShapeContent
            && (right_click_pos == ClickPosition::e_First)) {
            QMenu menu;
            QAction *action = new QAction(QObject::tr("Group"), &menu);
            action->setData(static_cast<int>(ShapeType::e_Group));
            menu.addAction(action);
            menu.addSeparator();
            action = new QAction(QObject::tr("Rectangle"), &menu);
            action->setData(static_cast<int>(ShapeType::e_Rectangle));
            menu.addAction(action);
            action = new QAction(QObject::tr("Ellipse"), &menu);
            action->setData(static_cast<int>(ShapeType::e_Ellipse));
            menu.addAction(action);
            action = new QAction(QObject::tr("Polystar"), &menu);
            action->setData(static_cast<int>(ShapeType::e_Star));
            menu.addAction(action);
            action = new QAction(QObject::tr("Path"), &menu);
            action->setData(static_cast<int>(ShapeType::e_Shape));
            menu.addAction(action);

            menu.addSeparator();
            action = new QAction(QObject::tr("Fill"), &menu);
            action->setData(static_cast<int>(ShapeType::e_Fill));
            menu.addAction(action);
            action = new QAction(QObject::tr("Stroke"), &menu);
            action->setData(static_cast<int>(ShapeType::e_Stroke));
            menu.addAction(action);
            action = new QAction(QObject::tr("Gradient Fill"), &menu);
            action->setData(static_cast<int>(ShapeType::e_GradientFill));
            menu.addAction(action);
            action = new QAction(QObject::tr("Gradient Stroke"), &menu);
            action->setData(static_cast<int>(ShapeType::e_GradientStroke));
            menu.addAction(action);

            menu.addSeparator();
            action = new QAction(QObject::tr("Trim Paths"), &menu);
            action->setData(static_cast<int>(ShapeType::e_Trim));
            menu.addAction(action);

            action = new QAction(QObject::tr("Repeater"), &menu);
            action->setData(static_cast<int>(ShapeType::e_Repeater));
            menu.addAction(action);

            // if content selected, add only to content
            // else find groups for all the selected items which belongs to same layer
            // add to it, for the time being only adding to content group
            QAction *act = menu.exec(ev->globalPosition().toPoint());
            if (act) {
                model->setData(index, act->data(), CompositionDocument::AddShapeItem);
                return true;
            }

        } else if (value.m_type == EditColumnValue::Color
                   && (left_click_pos == ClickPosition::e_First)) {
            QColor prev_color = value.m_value.value<QColor>();
            QColorDialog dialog(prev_color);
            auto set_color = [this, &index](const QColor &color) {
                EditColumnValue val;
                val.m_type = EditValueType::Color;
                val.m_value = color;
                QVariant var;
                var.setValue(val);
                m_d->m_model->setData(index, var, CompositionDocument::EditValue);
            };
            QObject::connect(&dialog, &QColorDialog::currentColorChanged, &dialog, set_color);
            int result = dialog.exec();
            if (result == QDialog::Rejected) {
                set_color(prev_color);
            }
            return true;
        } else if (value.m_type == EditColumnValue::PathDirection
                   && (left_click_pos == ClickPosition::e_First)) {
            EditColumnValue val;
            val.m_type = EditValueType::PathDirection;
            val.m_value = !value.m_value.toBool();
            QVariant var;
            var.setValue(val);
            m_d->m_model->setData(index, var, CompositionDocument::EditValue);
            return true;
        } else if (value.m_type == EditColumnValue::PropertyText
                   && index.data(CompositionDocument::IsLinkable).toBool()
                   && (left_click_pos == ClickPosition::e_First)) {
            m_d->m_model->setData(index,
                                  !index.data(CompositionDocument::IsLinked).toBool(),
                                  CompositionDocument::SetLinked);
            return true;
        } else if (value.m_type == EditColumnValue::LayerProperties) {
            QVariantList data = value.m_value.toList();
            Q_ASSERT(data.size() == EditColumnValue::PropLast);
            ClickPosition click_position = column_position_for_click(ev->position(), option.rect);
            if (click_position == ClickPosition::e_First) {
                // blend mode
                const QStringList &blend_modes = Utility::instance()->blend_modes();
                int blend = data[EditColumnValue::BlendMode].toInt();
                QMenu menu;
                QAction *def = nullptr;
                for (int i = 0; i < blend_modes.size(); ++i) {
                    QAction *act = new QAction(blend_modes[i], &menu);
                    act->setCheckable(true);
                    act->setData(i);
                    if (i == blend) {
                        act->setChecked(true);
                        def = act;
                    }
                    menu.addAction(act);
                }
                QAction *act = menu.exec(ev->globalPosition().toPoint(), def);
                if (act) {
                    int new_blend = act->data().toInt();
                    if (new_blend != blend) {
                        m_d->m_model->setData(index, new_blend, CompositionDocument::SetBlendMode);
                        return true;
                    }
                }
            } else if (m_d->document->show_matte_details()) {
                if (click_position == ClickPosition::e_Second) {
                    // matte mode
                    const QStringList &matte_modes = Utility::instance()->matte_modes();
                    int matte = data[EditColumnValue::MatteType].toInt();
                    QMenu menu;
                    QAction *def = nullptr;
                    for (int i = 0; i < matte_modes.size(); ++i) {
                        QAction *act = new QAction(matte_modes[i], &menu);
                        act->setCheckable(true);
                        act->setData(i);
                        if (i == matte) {
                            act->setChecked(true);
                            def = act;
                        }
                        menu.addAction(act);
                    }
                    QAction *act = menu.exec(ev->globalPosition().toPoint(), def);
                    if (act) {
                        int new_matte = act->data().toInt();
                        if (new_matte != matte) {
                            m_d->m_model->setData(index,
                                                  new_matte,
                                                  CompositionDocument::SetMatteType);
                            return true;
                        }
                    }
                } else if (click_position == ClickPosition::e_Third
                           || click_position == ClickPosition::e_Fourth) {
                    // matte layer
                    const auto &layer_to_name_map = m_d->document->layer_index_to_name_map();
                    int layer = click_position == ClickPosition::e_Third
                                    ? data[EditColumnValue::MatteLayerIndex].toInt()
                                    : data[EditColumnValue::ParentLayerIndex].toInt();
                    QMenu menu;
                    QAction *def = nullptr;
                    for (const auto &item : layer_to_name_map) {
                        QAction *act = new QAction(item.second, &menu);
                        act->setCheckable(true);
                        act->setData(item.first);
                        if (item.first == layer) {
                            act->setChecked(true);
                            def = act;
                        }
                        menu.addAction(act);
                    }
                    QAction *act = menu.exec(ev->globalPosition().toPoint(), def);
                    if (act) {
                        int new_layer = act->data().toInt();
                        if (new_layer != layer) {
                            m_d->m_model->setData(index,
                                                  new_layer,
                                                  click_position == ClickPosition::e_Third
                                                      ? CompositionDocument::SetMatteLayer
                                                      : CompositionDocument::SetParentLayer);
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }

    bool mouse_move(QMouseEvent *ev,
                    QAbstractItemModel *model,
                    const QStyleOptionViewItem &option,
                    const QModelIndex &index) override
    {
        if (m_d->m_is_handling_timeline) {
            return false;
        }

        using namespace alive::gui;
        EditColumnValue value
            = model->data(index, CompositionDocument::EditValue).value<EditColumnValue>();
        QModelIndex old_index;
        bool reset_hover_state = false;
        if (value.m_type == EditColumnValue::PropertyText) {
            // hover event
            const auto *sizes = alive::theme::TimelineSize::instance();
            if (m_d->current_field != alive::EditingField::e_None) {
                old_index = m_d->current_index;
            }

            m_d->current_field
                = m_d->field_from_position(value.m_value, index, ev->position(), option, sizes);
            if (m_d->current_field != alive::EditingField::e_None) {
                m_d->current_index = index;
                m_d->editing_context = EditingContext::e_Hover;
                m_d->m_cursor = alive::gui::CursorManager::instance()->horizontal_size_cursor();
            } else {
                reset_hover_state = true;
            }
        } else {
            // on other property type
            reset_hover_state = true;
        }

        if (reset_hover_state) {
            m_d->current_index = QModelIndex();
            m_d->editing_context = EditingContext::e_None;
            m_d->restore_cursor();
        }
        return true;
    }
};

EditColumnDelegate::EditColumnDelegate(const alive::model::CompositionDocument *doc,
                                       QAbstractItemModel *model,
                                       QObject *parent)
    : QAbstractItemDelegate{parent}
    , m_d(std::make_unique<EditColumnDelegatePrivateData>(doc, model, this))
{
    m_d->m_height = alive::theme::TimelineSize::instance()->item_height();
    m_d->current_state.reset(std::make_unique<NormalState>(m_d.get()).release());
}

EditColumnDelegate::~EditColumnDelegate() {}

QSize EditColumnDelegate::sizeHint(const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
    return QSize(100, m_d->m_height);
}

void EditColumnDelegate::paint(QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    const auto *colors = alive::theme::ColorPalette::instance();
    const auto *sizes = alive::theme::TimelineSize::instance();
    painter->save();
    using namespace alive::model;
    EditColumnValue value
        = m_d->m_model->data(index, CompositionDocument::EditValue).value<EditColumnValue>();
    switch (value.m_type) {
    case EditColumnValue::ShapeContent:
        draw_shape_content(painter, option, colors, sizes);
        break;
    case EditColumnValue::Color:
        draw_color(value.m_value.value<QColor>(), painter, option, colors, sizes);
        break;
    case EditColumnValue::PropertyText:
        draw_property_text(value.m_value, index, painter, option, colors, sizes);
        break;
    case EditColumnValue::PathDirection:
        draw_path_text(value.m_value.toBool(), painter, option, colors, sizes);
        break;
    case EditColumnValue::LayerProperties: {
        draw_layer_properties(value.m_value, painter, option, colors, sizes);
    } break;
    default:
        break;
    }
    painter->restore();
}

bool EditColumnDelegate::editorEvent(QEvent *event,
                                     QAbstractItemModel *model,
                                     const QStyleOptionViewItem &option,
                                     const QModelIndex &index)
{
    using namespace alive::model;
    QMouseEvent *me = nullptr;
    const auto ev_type = event->type();
    if (!m_d->m_is_handling_timeline) {
        if (ev_type == QEvent::MouseMove || ev_type == QEvent::MouseButtonPress
            || ev_type == QEvent::MouseButtonRelease) {
            me = static_cast<QMouseEvent *>(event);
        }
    }

    bool accept = false;
    if (me) {
        if (ev_type == QEvent::MouseMove) {
            accept = m_d->current_state->mouse_move(me, model, option, index);
        } else if (ev_type == QEvent::MouseButtonPress) {
            accept = m_d->current_state->mouse_press(me, model, option, index);
        } else if (ev_type == QEvent::MouseButtonRelease) {
            accept = m_d->current_state->mouse_release(me, model, option, index);
        }
    }

    if (accept) {
        event->accept();
    }
    return accept;
}

bool EditColumnDelegate::is_dragging() const
{
    return m_d->editing_context == EditingContext::e_Dragging;
}

void EditColumnDelegate::stop_dragging()
{
    if (m_d->editing_context != EditingContext::e_Dragging) {
        return;
    }

    m_d->restore_cursor();
    m_d->editing_context = EditingContext::e_None;
    m_d->current_field = alive::EditingField::e_None;
    m_d->current_state.reset(std::make_unique<NormalState>(m_d.get()).release());
    emit m_d->m_model->dataChanged(m_d->current_index, m_d->current_index);
}

void EditColumnDelegate::drag(QPointF p)
{
    m_d->drag(p);
}

void EditColumnDelegate::request_editor(QModelIndex index)
{
    emit invoke_open_editor(index);
}

void EditColumnDelegate::request_close_editor(QWidget *editor)
{
    emit invoke_close_editor(editor);
}

bool EditColumnDelegate::eventFilter(QObject *obj, QEvent *event)
{
    if (m_d->editing_context != EditingContext::e_Editing)
        return false;

    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *ev = static_cast<QKeyEvent *>(event);
        const int key = ev->key();
        if (key == Qt::Key_Escape) {
            m_d->reset_from_edit_state(static_cast<QWidget *>(obj));
            return true;
        }

        if (key == Qt::Key_Enter || key == Qt::Key_Return) {
            QLineEdit *le = static_cast<QLineEdit *>(obj);
            QString text;
            if (le) {
                text = le->text();
            }
            int role = m_d->current_field == EditingField::e_First
                           ? alive::gui::CompositionDocument::SetValueTextT1
                           : alive::gui::CompositionDocument::SetValueTextT2;
            m_d->reset_from_edit_state(static_cast<QWidget *>(obj));
            m_d->m_model->setData(m_d->current_index, text, role);
            return true;
        }
    } else if (event->type() == QEvent::FocusOut) {
        m_d->reset_from_edit_state(static_cast<QWidget *>(obj));
    }
    return false;
}

void EditColumnDelegate::draw_property_text(const QVariant &var_value,
                                            const QModelIndex &index,
                                            QPainter *painter,
                                            const QStyleOptionViewItem &option,
                                            const alive::theme::ColorPalette *colors,
                                            const alive::theme::TimelineSize *sizes) const
{
    alive::gui::PropertyTextValue prop_text = var_value.value<alive::gui::PropertyTextValue>();
    // draw t1
    QRectF rect(option.rect);

    if (index.data(model::CompositionDocument::IsLinkable).toBool()) {
        int icon_height = sizes->icon_size().height();
        QRect icon_rect(rect.left() + sizes->item_spacing(),
                        rect.top() + (m_d->m_height - icon_height) / 2,
                        sizes->icon_size().width(),
                        icon_height);
        const auto *icon_mgr = IconManager::instance();
        painter->fillRect(icon_rect, colors->primary_background_color());
        if (index.data(model::CompositionDocument::IsLinked).toBool()) {
            icon_mgr->get_icon(IconManager::e_Link).paint(painter, icon_rect);
        }
        rect.moveLeft(icon_rect.right());
    }

    rect.moveLeft(rect.left() + sizes->item_spacing());
    QFont font = QApplication::font();
    QFontMetrics fmc(font);
    rect.setHeight(fmc.height());
    rect.moveCenter(QPointF(rect.center().x(), option.rect.center().y()));
    bool m_expr_enabled = index.data(model::CompositionDocument::ExprEnabled).toBool();
    QRectF text_rect;
    bool editing_first = false;
    if (m_d->current_field == alive::EditingField::e_First && m_d->current_index == index) {
        if (m_d->editing_context == EditingContext::e_Editing) {
            editing_first = true;
        }
        painter->setPen(colors->bright_color());
    } else {
        painter->setPen(m_expr_enabled ? colors->play_cursor_color().lighter()
                                       : colors->accent_color());
    }
    painter->drawText(rect, Qt::AlignLeft, prop_text.t1, &text_rect);
    if (editing_first) {
        text_rect.setRight(rect.left() + k_prop_editor_width);
    }

    if (prop_text.t2.length()) {
        rect.moveLeft(text_rect.right());
        painter->setPen(colors->primary_font_color());
        // draw seperator
        painter->drawText(rect, Qt::AlignLeft, prop_text.seperator, &text_rect);

        rect.moveLeft(text_rect.right());
        bool editing_second = false;
        if (m_d->current_field == alive::EditingField::e_Second && m_d->current_index == index) {
            if (m_d->editing_context == EditingContext::e_Editing) {
                editing_second = true;
            }
            painter->setPen(colors->bright_color());
        } else {
            painter->setPen(m_expr_enabled ? colors->play_cursor_color().lighter()
                                           : colors->accent_color());
        }
        painter->drawText(rect, Qt::AlignLeft, prop_text.t2, &text_rect);
        if (editing_second) {
            text_rect.setRight(rect.left() + k_prop_editor_width);
        }
    }
    if (prop_text.suffix.length()) {
        rect.moveLeft(text_rect.right());
        painter->setPen(colors->primary_font_color());
        painter->drawText(rect, Qt::AlignLeft, prop_text.suffix, &text_rect);
    }
    // draw seperator
}

void EditColumnDelegate::draw_path_text(bool value,
                                        QPainter *painter,
                                        const QStyleOptionViewItem &option,
                                        const alive::theme::ColorPalette *colors,
                                        const alive::theme::TimelineSize *sizes) const
{
    // draw t1
    QRectF rect(option.rect);
    int icon_height = sizes->icon_size().height();
    QRect icon_rect(rect.left() + sizes->item_spacing(),
                    rect.top() + (m_d->m_height - icon_height) / 2,
                    sizes->icon_size().width(),
                    icon_height);
    const auto *icon_mgr = IconManager::instance();
    if (value) {
        icon_mgr->get_icon(IconManager::e_BoxChecked).paint(painter, icon_rect);
    } else {
        icon_mgr->get_icon(IconManager::e_BoxUnChecked).paint(painter, icon_rect);
    }

    QFont font = QApplication::font();
    QFontMetrics fmc(font);
    rect.setHeight(fmc.height());
    rect.moveLeft(option.rect.left() + sizes->item_spacing());
    rect.moveCenter(QPointF(rect.center().x(), option.rect.center().y()));
    rect.setLeft(icon_rect.right() + sizes->item_spacing());
    painter->drawText(rect, tr("Clockwise"));
}

void EditColumnDelegate::draw_layer_properties(QVariant value,
                                               QPainter *painter,
                                               const QStyleOptionViewItem &option,
                                               const alive::theme::ColorPalette *colors,
                                               const alive::theme::TimelineSize *sizes) const
{
    const IconManager *icon_mgr = IconManager::instance();
    using namespace alive::model;
    QVariantList data = value.toList();
    Q_ASSERT(data.size() == EditColumnValue::PropLast);

    int blend = data[EditColumnValue::BlendMode].toInt();
    int matte = data[EditColumnValue::MatteType].toInt();
    int matte_index = data[EditColumnValue::MatteLayerIndex].toInt();
    int parent_index = data[EditColumnValue::ParentLayerIndex].toInt();
    const auto *utility = alive::gui::Utility::instance();
    const QString &blend_str = utility->blend_modes()[blend];
    QRect rect(option.rect);

    rect.setRight(m_d->document->show_matte_details() ? rect.left() + sizes->column_width()
                                                      : rect.right());
    draw_combobox_from_item_view(painter, blend_str, rect, colors, sizes, icon_mgr);
    if (m_d->document->show_matte_details()) {
        rect.moveLeft(rect.right());
        draw_combobox_from_item_view(painter,
                                     utility->matte_mode(matte),
                                     rect,
                                     colors,
                                     sizes,
                                     icon_mgr);
        rect.moveLeft(rect.right());
        draw_combobox_from_item_view(painter,
                                     m_d->document->layer_name(matte_index),
                                     rect,
                                     colors,
                                     sizes,
                                     icon_mgr);
        rect.moveLeft(rect.right());
        draw_combobox_from_item_view(painter,
                                     m_d->document->layer_name(parent_index),
                                     rect,
                                     colors,
                                     sizes,
                                     icon_mgr);
    }
}

void EditColumnDelegate::set_handling_timeline(bool flag)
{
    m_d->m_is_handling_timeline = flag;
}

QWidget *EditColumnDelegate::createEditor(QWidget *parent,
                                          const QStyleOptionViewItem &option,
                                          const QModelIndex &index) const
{
    if (m_d->editing_context != EditingContext::e_Editing)
        return nullptr;

    using namespace alive::model;
    EditColumnValue value
        = m_d->m_model->data(index, CompositionDocument::EditValue).value<EditColumnValue>();
    switch (value.m_type) {
    // case EditColumnValue::PathDirection: {
    //     QComboBox *combobox = new QComboBox(parent);
    //     combobox->setAutoFillBackground(true);
    //     combobox->addItem(tr("CounterClockwise"));
    //     combobox->addItem(tr("Clockwise"));
    //     combobox->installEventFilter(const_cast<EditColumnDelegate *>(this));
    //     return combobox;
    // } break;
    case EditColumnValue::PropertyText: {
        QLineEdit *line_edit = new QLineEdit(parent);
        line_edit->installEventFilter(const_cast<EditColumnDelegate *>(this));
        return line_edit;
    } break;
    default:
        break;
    }

    return nullptr;
}

void EditColumnDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (m_d->editing_context != EditingContext::e_Editing)
        return;

    using namespace alive::model;
    EditColumnValue value
        = m_d->m_model->data(index, CompositionDocument::EditValue).value<EditColumnValue>();
    if (value.m_type == EditColumnValue::PropertyText) {
        QLineEdit *le = static_cast<QLineEdit *>(editor);
        alive::gui::PropertyTextValue prop_text = m_d->m_model
                                                     ->data(index, CompositionDocument::EditValue)
                                                     .value<EditColumnValue>()
                                                     .m_value.value<alive::gui::PropertyTextValue>();
        if (m_d->current_field == EditingField::e_First) {
            le->setText(prop_text.t1);
        } else if (m_d->current_field == EditingField::e_Second) {
            le->setText(prop_text.t2);
        }
    }
}

void EditColumnDelegate::updateEditorGeometry(QWidget *editor,
                                              const QStyleOptionViewItem &option,
                                              const QModelIndex &index) const
{
    if (m_d->editing_context != EditingContext::e_Editing)
        return;

    if (editor) {
        QRect rect = option.rect;
        const auto *sizes = alive::theme::TimelineSize::instance();
        auto spacing = sizes->item_spacing();
        if (index.data(model::CompositionDocument::IsLinkable).toBool()) {
            spacing += sizes->icon_size().width() + sizes->item_spacing();
        }
        rect.setLeft(rect.left() + spacing);
        if (m_d->current_field == EditingField::e_First) {
            rect.setWidth(k_prop_editor_width);
        } else if (m_d->current_field == EditingField::e_Second) {
            using namespace alive::model;
            alive::gui::PropertyTextValue prop_text
                = m_d->m_model->data(index, CompositionDocument::EditValue)
                      .value<EditColumnValue>()
                      .m_value.value<alive::gui::PropertyTextValue>();
            // draw t1
            QFont font = QApplication::font();
            QFontMetrics fmc(font);
            rect.setLeft(rect.left() + fmc.horizontalAdvance(prop_text.t1 + prop_text.seperator));
            rect.setWidth(k_prop_editor_width);
        }
        editor->setGeometry(rect);
    }
}

bool DragState::mouse_release(QMouseEvent *ev,
                              QAbstractItemModel *model,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index)
{
    if (m_d->m_is_handling_timeline) {
        return false;
    }

    if (m_d->editing_context == EditingContext::e_DragStarted) {
        m_d->editing_context = EditingContext::e_Editing;
        m_d->current_state.reset(std::make_unique<EditState>(m_d, index).release());
        return true;
    } else if (m_d->editing_context == EditingContext::e_Dragging) {
        m_d->editing_context = EditingContext::e_None;
        // change to normal state
        m_d->current_state.reset(std::make_unique<NormalState>(m_d).release());
        return true;
    }
    return true;
}

void EditColumnDelegatePrivateData::reset_from_edit_state(QWidget *editor)
{
    editing_context = EditingContext::e_None;
    current_field = EditingField::e_None;
    current_state.reset(std::make_unique<NormalState>(this).release());
    object->request_close_editor(editor);
}

} // namespace alive
