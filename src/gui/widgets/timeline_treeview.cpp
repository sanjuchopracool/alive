#include "timeline_treeview.h"
#include <core/constants/file_extension.h>
#include <core/model/composition.h>
#include <core/model/keyframes/keyframe.h>
#include <core/model/tree_item.h>
#include <core/serializer/editor_support.h>
#include <core/serializer/lottie_parser.h>
#include <core/tagging/tag_manager.h>
#include <gui/cursor_manager.h>
#include <gui/delegates/edit_column_delegate.h>
#include <gui/delegates/feature_column_delegate.h>
#include <gui/delegates/layer_column_delegate.h>
#include <gui/delegates/timeline_column_delegate.h>
#include <gui/delegates/tree_column_delegate.h>
#include <gui/document/composition_document.h>
#include <gui/document/composition_document_proxy_model.h>
#include <editor/undo_support/document_undo_manager.h>
#include <gui/framework/theme/color_palette.h>
#include <gui/framework/widgets/bezier_curve_editor.h>
#include <gui/framework/widgets/timeline_expression_editor.h>
#include <gui/project/project_manager.h>
#include <gui/quick/range_selector_helper.h>
#include <gui/settings/user_settings.h>
#include <gui/theme/timeline_style.h>
#include <gui/timeline_model_helpers.h>
#include <gui/utility/keyframe_selection_manager.h>
#include <gui/widgets/timeline_range_selector.h>
#include <QDrag>
#include <QFileDialog>
#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QScrollBar>

namespace {
QMenu *g_layer_context_menu = nullptr;
QMenu *g_object_context_menu = nullptr;
QMenu *g_property_and_keyframe_context_menu = nullptr;
QMenu *g_keyframe_add_tag_menu = nullptr;
QMenu *g_keyframe_remove_tag_menu = nullptr;
QAction *g_add_tag_action = nullptr;
QAction *g_remove_tag_action = nullptr;

alive::model::Layer *g_layer_in_context = nullptr;
alive::model::Object *g_object_in_context = nullptr;
alive::model::Property *g_property_in_context = nullptr;
alive::model::KeyFrame *g_keyframe_in_context = nullptr;
alive::model::CompositionDocument *g_doc_in_context = nullptr;
QModelIndex g_index_in_context;
alive::TimelineTreeView *g_view_in_context = nullptr;

bool g_init_done = false;
void init_layer_menu()
{
    if (g_init_done) {
        return;
    }
    g_layer_context_menu = new QMenu(QObject::tr("Layer Menu"));
    QAction *action = new QAction(QObject::tr("&Setting"), g_layer_context_menu);
    g_layer_context_menu->addAction(action);
    QObject::connect(action, &QAction::triggered, g_layer_context_menu, [=]() {
        alive::project::ProjectManager::instance()->invoke_setting_for_layer(g_layer_in_context);
    });

    action = new QAction(QObject::tr("&Rename"), g_layer_context_menu);
    g_layer_context_menu->addAction(action);
    QObject::connect(action, &QAction::triggered, g_layer_context_menu, [=]() {
        g_view_in_context->edit(g_index_in_context);
    });

    action = new QAction(QObject::tr("&Delete"), g_layer_context_menu);
    g_layer_context_menu->addAction(action);
    QObject::connect(action, &QAction::triggered, g_layer_context_menu, [=]() {
        g_doc_in_context->delete_item(g_layer_in_context);
    });

    action = new QAction(QObject::tr("&Show Expression Editor"), g_layer_context_menu);
    g_layer_context_menu->addAction(action);
    QObject::connect(action, &QAction::triggered, g_layer_context_menu, [=]() {
        g_view_in_context->show_expression_editor(true);
    });

    action = new QAction(QObject::tr("&Hide Expression Editor"), g_layer_context_menu);
    g_layer_context_menu->addAction(action);
    QObject::connect(action, &QAction::triggered, g_layer_context_menu, [=]() {
        g_view_in_context->show_expression_editor(false);
    });

    g_object_context_menu = new QMenu(QObject::tr("Object Menu"));
    action = new QAction(QObject::tr("&Rename"), g_object_context_menu);
    g_object_context_menu->addAction(action);
    QObject::connect(action, &QAction::triggered, g_object_context_menu, [=]() {
        g_view_in_context->edit(g_index_in_context);
    });

    action = new QAction(QObject::tr("&Delete"), g_object_context_menu);
    g_object_context_menu->addAction(action);
    QObject::connect(action, &QAction::triggered, g_object_context_menu, [=]() {
        g_doc_in_context->delete_item(g_object_in_context);
    });

    {
        g_property_and_keyframe_context_menu = new QMenu(QObject::tr("Property Menu"));
        action = new QAction(QObject::tr("&Set Expression"), g_property_and_keyframe_context_menu);
        g_property_and_keyframe_context_menu->addAction(action);
        QObject::connect(action, &QAction::triggered, g_object_context_menu, [=]() {
            g_view_in_context->set_expression();
        });

        action = new QAction(QObject::tr("&Disable Expression"),
                             g_property_and_keyframe_context_menu);
        g_property_and_keyframe_context_menu->addAction(action);
        QObject::connect(action, &QAction::triggered, g_object_context_menu, [=]() {
            g_view_in_context->disable_expression();
        });

        g_keyframe_add_tag_menu = new QMenu(QObject::tr("&Add Tag"),
                                            g_property_and_keyframe_context_menu);

        g_keyframe_remove_tag_menu = new QMenu(QObject::tr("&Remove Tag"),
                                               g_property_and_keyframe_context_menu);

        auto add_tag_actions = [=]() {
            g_keyframe_add_tag_menu->clear();
            g_keyframe_remove_tag_menu->clear();

            if (g_keyframe_in_context && g_doc_in_context) {
                Taggable *taggable = g_keyframe_in_context;
                TagManager *mgr = g_doc_in_context->composition()->tag_manager();
                if (taggable && mgr) {
                    for (auto *tag : *taggable) {
                        g_keyframe_remove_tag_menu
                            ->addAction(QString::fromStdString(tag->name()), [tag]() {
                                g_doc_in_context->undo_manager()->remove_tag(g_keyframe_in_context,
                                                                             tag);
                            });
                    }
                    for (auto &tag_ptr : *mgr) {
                        Tag *tag = tag_ptr.get();
                        if (tag && !taggable->has_tag(tag)) {
                            g_keyframe_add_tag_menu
                                ->addAction(QString::fromStdString(tag->name()), [tag]() {
                                    g_doc_in_context->undo_manager()->tag(g_keyframe_in_context,
                                                                          tag);
                                });
                        }
                    }
                }
            };
        };
        QObject::connect(g_property_and_keyframe_context_menu,
                         &QMenu::aboutToShow,
                         g_object_context_menu,
                         [=]() {
                             if (g_keyframe_in_context) {
                                 g_add_tag_action = g_property_and_keyframe_context_menu->addMenu(
                                     g_keyframe_add_tag_menu);
                                 g_remove_tag_action = g_property_and_keyframe_context_menu->addMenu(
                                     g_keyframe_remove_tag_menu);
                                 add_tag_actions();
                             } else {
                                 g_property_and_keyframe_context_menu->removeAction(
                                     g_add_tag_action);
                                 g_property_and_keyframe_context_menu->removeAction(
                                     g_remove_tag_action);
                                 g_add_tag_action = nullptr;
                                 g_remove_tag_action = nullptr;
                             }
                         });
    }

    g_init_done = true;
}
} // namespace

namespace alive {

enum class MousePositionArea { e_None, e_LeftHandle, e_Range, e_RightHandle };

struct TimelineTreeViewPrivateData
{
    TimelineTreeViewPrivateData(alive::model::CompositionDocument *t_doc,
                                const TimelineRangeSelector *selector)
        : doc(t_doc)
        , model(t_doc->proxy_model())
        , range_selector(selector)
        , timeline_helper(selector->timeline_helper())
    {
        auto const &rect = alive::theme::TimelineSize::instance()->icon_size();
        arrow_path.moveTo(-rect.width() / 3, -rect.height() / 6);
        arrow_path.lineTo(0, rect.height() / 6);
        arrow_path.lineTo(rect.width() / 3, -rect.height() / 6);
    }
    alive::model::CompositionDocument *doc = nullptr;
    MousePositionArea mouse_pos_area = MousePositionArea::e_None;
    QAbstractItemModel *model = nullptr;
    const TimelineRangeSelector *range_selector = nullptr;
    const alive::gui::TimelineHelper *timeline_helper = nullptr;
    KeyFrameSelectionManager *keyframe_selection_mgr = nullptr;
    int m_cursor_id = 0;
    int timeline_column_offset = 0;
    bool is_timeline_dragging = false;
    bool is_keyframes_dragging = false;
    MousePositionArea timeline_drag_area = MousePositionArea::e_None;
    QModelIndex timeline_drag_index;
    qreal drag_pos_x;
    QPainterPath arrow_path;
    TreeColumnDelegate *tree_column_delegate = nullptr;
    EditColumnDelegate *edit_column_delegate = nullptr;
    FeatureColumnDelegate *feature_column_delegate = nullptr;
    QPoint drag_and_drop_start_pos;
    alive::BezierCurveEditor *bezier_editor = nullptr;
    alive::TimelineExpressionEditor *expression_editor = nullptr;

    // drag and drop support
    TreeItem *drag_and_drop_item = nullptr;

    void clear_keyframe_selection()
    {
        if (keyframe_selection_mgr->is_selected()) {
            keyframe_selection_mgr->slot_clear_all_keyframes_from_selection();
        }
    }

    void update_cursor(MousePositionArea area)
    {
        if (mouse_pos_area == area) {
            return;
        }

        auto cursor_manager = alive::gui::CursorManager::instance();
        if (area == MousePositionArea::e_None) {
            if (mouse_pos_area != MousePositionArea::e_None) {
                cursor_manager->restore_cursor(m_cursor_id);
            }
        } else {
            if (area == MousePositionArea::e_Range) {
                m_cursor_id = cursor_manager->horizontal_size_cursor();
            } else if (area == MousePositionArea::e_LeftHandle) {
                m_cursor_id = cursor_manager->left_drag_cursor();
            } else if (area == MousePositionArea::e_RightHandle) {
                m_cursor_id = cursor_manager->right_drag_cursor();
            }
        }
        mouse_pos_area = area;
    }

    MousePositionArea find_overlap_area(qreal x, int start_frame, int end_frame) const
    {
        MousePositionArea result = MousePositionArea::e_None;
        qreal range_x1 = timeline_helper->position_for_frame(start_frame);
        qreal range_x2 = timeline_helper->position_for_frame(end_frame);

        const int handle_width = alive::theme::TimelineSize::instance()->handle_width();
        if (x >= range_x1 && x <= range_x2) {
            if (x <= (range_x1 + handle_width)) {
                result = MousePositionArea::e_LeftHandle;
            } else if (x >= (range_x2 - handle_width)) {
                result = MousePositionArea::e_RightHandle;
            } else {
                result = MousePositionArea::e_Range;
            }
        }
        return result;
    }

    bool handle_left_mouse_click(const QModelIndex &index,
                                 QMouseEvent *ev,
                                 const QRect &visual_rect,
                                 bool double_click = false)
    {
        bool result = false;
        using namespace alive::model;
        bool clear_keyframes = false;
        qreal x = ev->position().x() - timeline_column_offset;
        if (x > 0) {
            drag_pos_x = x;
            bool is_layer = model->data(index, CompositionDocument::IsLayer).toBool();
            if (is_layer) {
                const LayerInOutPoints in_out_value
                    = model->data(index, CompositionDocument::LayerInOutRange)
                          .value<LayerInOutPoints>();
                timeline_drag_area = find_overlap_area(x,
                                                       in_out_value.in_point,
                                                       in_out_value.out_point);
                if (timeline_drag_area == MousePositionArea::e_None) {
                    is_timeline_dragging = false;
                } else {
                    is_timeline_dragging = true;
                    timeline_drag_index = index;
                    result = true;
                }

                clear_keyframes = true;
            } else {
                // object or property
                if (model->data(index, CompositionDocument::IsAnimating).toBool()) {
                    // check for selection of keyframe
                    Property *property = nullptr;
                    alive::TreeItem *item = model->data(index, CompositionDocument::ItemRole)
                                                .value<alive::TreeItem *>();
                    if (item) {
                        property = item->to_property();
                    }
                    if (property) {
                        QPointF pos = ev->position();
                        pos.setX(x);
                        pos.setY(pos.y() - visual_rect.center().y());
                        result = keyframe_selection_mgr->clicked_at(property,
                                                           pos,
                                                           ev->modifiers() == Qt::ShiftModifier
                                                               || double_click);
                    }
                } else {
                    clear_keyframes = true;
                }
            }
        } else {
            clear_keyframes = true;
        }

        if (clear_keyframes) {
            result = keyframe_selection_mgr->is_selected();
            clear_keyframe_selection();
            is_keyframes_dragging = false;
        } else {
            is_keyframes_dragging = keyframe_selection_mgr->is_selected();
        }
        edit_column_delegate->set_handling_timeline(is_timeline_dragging || is_keyframes_dragging);
        result |= (is_timeline_dragging || is_keyframes_dragging);
        return result;
    }

    void handle_timeline_mouse_drag(qreal x)
    {
        x -= timeline_column_offset;
        if (x <= 0)
            return;

        if (is_timeline_dragging) {
            if (timeline_drag_area == MousePositionArea::e_LeftHandle) {
                model->setData(timeline_drag_index,
                               timeline_helper->frame_for_click(x),
                               alive::model::CompositionDocument::SetInPoint);
            } else if (timeline_drag_area == MousePositionArea::e_RightHandle) {
                model->setData(timeline_drag_index,
                               timeline_helper->frame_for_click(x),
                               alive::model::CompositionDocument::SetOutPoint);
            } else if (timeline_drag_area == MousePositionArea::e_Range) {
                qreal delta = x - drag_pos_x;
                int frame = static_cast<int>(delta / timeline_helper->width_per_frame());
                model->setData(timeline_drag_index,
                               frame,
                               alive::model::CompositionDocument::MoveInOutPoint);
                drag_pos_x += frame * timeline_helper->width_per_frame();
            }
        } else if (is_keyframes_dragging) {
            qreal delta = x - drag_pos_x;
            int frame = static_cast<int>(delta / timeline_helper->width_per_frame());
            drag_pos_x += frame * timeline_helper->width_per_frame();
            keyframe_selection_mgr->slot_frames_moved(frame);
        }
    }
    void stop_dragging()
    {
        is_timeline_dragging = false;
        is_keyframes_dragging = false;
        edit_column_delegate->set_handling_timeline(false);
        edit_column_delegate->stop_dragging();
    }

    void handle_drag(QPointF p)
    {
        if (edit_column_delegate->is_dragging()) {
            edit_column_delegate->drag(p);
        } else if (is_timeline_dragging || is_keyframes_dragging) {
            handle_timeline_mouse_drag(p.x());
        }
    }
    bool is_dragging()
    {
        return is_timeline_dragging || edit_column_delegate->is_dragging() || is_keyframes_dragging;
    }
};

TimelineTreeView::TimelineTreeView(alive::model::CompositionDocument *doc,
                                   const TimelineRangeSelector *range_selector,
                                   QWidget *parent)
    : QTreeView(parent)
    , m_d(std::make_unique<TimelineTreeViewPrivateData>(doc, range_selector))
{
    auto helper = range_selector->timeline_helper();
    setMouseTracking(true);
    // TODO FIX SCROLL BAR
    horizontalScrollBar()->setEnabled(false);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    // hide header
    setHeaderHidden(true);

    setEditTriggers(EditTrigger::DoubleClicked);
    const auto *sizes = alive::theme::TimelineSize::instance();
    setIndentation(sizes->icon_size().width() + sizes->item_spacing());
    setExpandsOnDoubleClick(false);

    setUniformRowHeights(true);
    const int timeline_column = static_cast<int>(alive::model::CompositionDocument::e_TimeLine);
    setItemDelegateForColumn(timeline_column,
                             new TimelineColumnDelegate(doc->proxy_model(), helper, this));
    const int feature_column = static_cast<int>(alive::model::CompositionDocument::e_Features);
    setItemDelegateForColumn(feature_column,
                             m_d->feature_column_delegate
                             = new FeatureColumnDelegate(doc->proxy_model(), this));

    m_d->tree_column_delegate = new TreeColumnDelegate(doc->proxy_model(), this);

    // bezier
    {
        m_d->bezier_editor = new alive::BezierCurveEditor(this);
        m_d->bezier_editor->hide();
        m_d->bezier_editor->raise();
        m_d->bezier_editor->set_margin(20);
    }

    {
        m_d->expression_editor = new alive::TimelineExpressionEditor(this);
        m_d->expression_editor->hide();
        m_d->expression_editor->raise();
    }
    const int tree_column = static_cast<int>(alive::model::CompositionDocument::e_Tree);
    setItemDelegateForColumn(tree_column, m_d->tree_column_delegate);

    // const int layer_column = static_cast<int>(alive::model::CompositionDocument::e_Layer);
    // setItemDelegateForColumn(layer_column, new LayerColumnDelegate(doc->proxy_model(), this));
    // hideColumn(layer_column);

    const int edit_column = static_cast<int>(alive::model::CompositionDocument::e_Edit);
    setItemDelegateForColumn(edit_column,
                             m_d->edit_column_delegate = new EditColumnDelegate(doc,
                                                                                doc->proxy_model(),
                                                                                this));
    connect(m_d->edit_column_delegate,
            &EditColumnDelegate::invoke_open_editor,
            this,
            [this](const QModelIndex &index) { edit(index); });

    connect(m_d->edit_column_delegate,
            &EditColumnDelegate::invoke_close_editor,
            this,
            [this](QWidget *editor) { closeEditor(editor, QAbstractItemDelegate::NoHint); });

    setModel(doc->proxy_model());
    connect(m_d->feature_column_delegate,
            &FeatureColumnDelegate::go_to_time,
            helper->timeline(),
            &alive::gui::TimeLine::select_frame);

    m_d->keyframe_selection_mgr = new KeyFrameSelectionManager(doc, helper, this);
    connect(m_d->keyframe_selection_mgr,
            &KeyFrameSelectionManager::updated_selection,
            this,
            [this]() { viewport()->update(); });

    connect(m_d->keyframe_selection_mgr,
            &KeyFrameSelectionManager::show_bezier_editor,
            this,
            [this, helper, range_selector](model::KeyFrame *left, model::KeyFrame *right) {
                qreal x1 = helper->position_for_frame(left->time());
                qreal x2 = helper->position_for_frame(right->time());
                auto margin = m_d->bezier_editor->margin();
                QRect rect(range_selector->x() - margin + x1, 0, x2 - x1 + 2 * margin, height());
                m_d->bezier_editor->setGeometry(rect);
                m_d->bezier_editor->set_keyframes(left, right);
                m_d->bezier_editor->show();
                m_d->bezier_editor->reset_zoom();
                m_d->bezier_editor->setFocus();
            });

    connect(m_d->bezier_editor,
            &alive::BezierCurveEditor::easing_data_changed,
            this,
            [this](alive::model::EasingBetweenTwoFramesData *data) {
                m_d->doc->slot_edited_easing_between(*data);
            });

    connect(m_d->expression_editor,
            &alive::TimelineExpressionEditor::set_keyframe_expression,
            this,
            [this](alive::model::KeyFrame *keyframe, QString expr) {
                m_d->doc->undo_manager()->set_expression_on_property(keyframe->property(), expr.toStdString(), keyframe->time());
                m_d->expression_editor->hide();
            });
    connect(m_d->expression_editor,
            &alive::TimelineExpressionEditor::set_property_expression,
            this,
            [this](alive::model::Property* property, QString expr) {
                m_d->doc->undo_manager()->set_expression_on_property(property, expr.toStdString());
                m_d->expression_editor->hide();
            });
    connect(m_d->expression_editor,
            &alive::TimelineExpressionEditor::playground_expression_changed,
            this,
            [this](QString expr) { m_d->doc->set_playground_script(expr.toStdString()); });

    // enable drag and drop

    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setDragEnabled(true);
    viewport()->setAcceptDrops(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::InternalMove);

    // selection related
    connect(this->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &TimelineTreeView::slot_selection_changed);

    connect(helper->timeline(),
            &alive::gui::TimeLine::selected_frame_changed,
            this,
            &TimelineTreeView::slot_selection_changed);
}

TimelineTreeView::~TimelineTreeView() {}

void TimelineTreeView::adjust_after_column_resize(int layer_column_width)
{
    const int layer_column = static_cast<int>(alive::model::CompositionDocument::e_Layer);
    m_d->tree_column_delegate->set_layer_column_widht(layer_column_width);
    const int tree_column = static_cast<int>(alive::model::CompositionDocument::e_Tree);
    int width = columnWidth(tree_column) + layer_column_width;
    setColumnWidth(tree_column, width);
    hideColumn(layer_column);

    const int column = static_cast<int>(alive::model::CompositionDocument::e_TimeLine);
    m_d->timeline_column_offset = columnViewportPosition(column);
    viewport()->update();
}

void TimelineTreeView::set_expression()
{
    if (g_view_in_context != this)
        return;

    if (g_keyframe_in_context || g_property_in_context) {
        update_expression_editor_geometry();
    }
    if (g_keyframe_in_context) {
        m_d->expression_editor->open_editor(g_keyframe_in_context);
    } else if (g_property_in_context) {
        m_d->expression_editor->open_editor(g_property_in_context);
    }
}

void TimelineTreeView::disable_expression()
{
    if (g_keyframe_in_context) {
        m_d->doc->undo_manager()->disable_keyframe_expression(g_keyframe_in_context, true);
    } else if (g_property_in_context) {
        m_d->doc->undo_manager()->disable_property_expression(g_property_in_context, true);
    }
}

void TimelineTreeView::show_expression_editor(bool show)
{
    if (show) {
        QRect rect(m_d->range_selector->x(), 0, width() - m_d->range_selector->x(), height());
        update_expression_editor_geometry();
        m_d->expression_editor->open_editor(QString::fromStdString(m_d->doc->playground_script()));
    } else {
        m_d->expression_editor->hide();
    }
}

void TimelineTreeView::keyPressEvent(QKeyEvent *ev)
{
    // delete
    auto selected_items = [this]() {
        auto indexes = selectedIndexes();

        std::set<TreeItem *> items;
        for (const auto &index : indexes) {
            if (index.data(alive::model::CompositionDocument::CanDelete).toBool()) {
                auto item = index.data(alive::model::CompositionDocument::ItemRole)
                                .value<alive::TreeItem *>();
                if (item) {
                    items.insert(item);
                }
            }
        }
        return items;
    };
    bool accept = false;
    if (ev->modifiers() == Qt::NoModifier && ev->key() == Qt::Key_Delete) {
        for (auto *item : selected_items()) {
            m_d->doc->delete_item(item);
        }
        accept = true;
    } else if (ev->modifiers() == Qt::CTRL && ev->key() == Qt::Key_D) {
        m_d->doc->clone_items(selected_items());
        accept = true;
    }

    if (accept) {
        ev->accept();
        return;
    }
    QTreeView::keyPressEvent(ev);
}

void TimelineTreeView::enterEvent(QEnterEvent *ev)
{
    // qDebug() << "Enter Event";
    QTreeView::enterEvent(ev);
}

void TimelineTreeView::leaveEvent(QEvent *ev)
{
    // qDebug() << "Leave Event";
    m_d->update_cursor(MousePositionArea::e_None);

    QTreeView::leaveEvent(ev);
}

void TimelineTreeView::mousePressEvent(QMouseEvent *ev)
{
    bool accept = false;
    QModelIndex index = QTreeView::indexAt(ev->pos());
    if (ev->button() == Qt::LeftButton && index.isValid()) {
        accept = m_d->handle_left_mouse_click(index, ev, visualRect(index));
    }

    if (accept) {
        ev->accept();
        return;
    }

    const QPoint &pos = ev->globalPosition().toPoint();
    bool show_context_menu = false;
    // show context only in edit label column
    auto column = static_cast<model::CompositionItemModel::Section>(columnAt(ev->pos().x()));
    if (column == model::CompositionItemModel::e_Tree
        || column == model::CompositionItemModel::e_Layer
        || column == model::CompositionItemModel::e_TimeLine) {
        show_context_menu = true;
    }
    if (ev->button() == Qt::RightButton && show_context_menu) {
        auto [items, layers] = selected_items();

        bool timeline_column = column == model::CompositionItemModel::e_TimeLine;
        // atleaset two layer for precomposition
        if (!timeline_column && (layers.size() >= 2) && items.size() == layers.size()) {
            // only layers selected
            QMenu menu(this);
            auto precompose_action = menu.addAction(tr("Precompose"));
            if (menu.exec(pos) == precompose_action) {
                auto file_path = QFileDialog::getSaveFileName(
                    this,
                    tr("Save Layers as Precomposition"),
                    settings::UserSettings::instance()->last_opened_file_dir_url().toLocalFile(),
                    tr("Alive Precomposition (*.%1)").arg(file_extensions::k_alive));
                file_path = file_path.trimmed();
                if (!file_path.isEmpty()) {
                    auto uuid = QUuid::createUuid();
                    auto data = LottieParser::save_alive_pre_comp(
                        m_d->doc->composition(),
                        project::ProjectManager::instance()->settings_for_doc(m_d->doc),
                        layers,
                        uuid.toString(QUuid::WithoutBraces).toStdString());

                    if (!file_path.endsWith(file_extensions::k_alive)) {
                        file_path += ".";
                        file_path += file_extensions::k_alive;
                    }
                    QFile file(file_path);
                    if (file.open(QIODevice::WriteOnly)) {
                        file.write(data.c_str());

                        // delete layers from project
                        m_d->doc->delete_layers(layers);

                        // create precomp layer from saved file
                        file.flush();
                        file.close();
                        m_d->doc->create_precomp_layer(file_path.toStdString());
                    } else {
                        qWarning() << file.errorString();
                    }
                }
            }
            ev->accept();
            return;
        } else if (items.size() == 1) {
            // handle single context item
            alive::TreeItem *item = items.begin()->first;
            alive::model::Property *property = item->to_property();
            g_view_in_context = this;
            g_doc_in_context = m_d->doc;
            g_index_in_context = index;
            if (property) {
                bool execute_context = true;
                if (timeline_column) {
                    g_keyframe_in_context = m_d->keyframe_selection_mgr->only_selected_frame();
                    execute_context = g_keyframe_in_context != nullptr;
                } else {
                    g_keyframe_in_context = nullptr;
                }
                if (execute_context) {
                    g_property_in_context = property;
                    g_property_and_keyframe_context_menu->exec(pos);
                }
                ev->accept();
                return;
            } else if (!timeline_column) {
                auto layer = item->to_layer();
                auto obj = item->to_object();
                if (layer) {
                    g_layer_in_context = layer;
                    g_layer_context_menu->exec(pos);
                } else if (obj) {
                    if (obj->can_delete()) {
                        g_object_in_context = obj;
                        g_object_context_menu->exec(pos);
                    }
                }
                ev->accept();
                return;
            }
        }
    }
    QTreeView::mousePressEvent(ev);
}

void TimelineTreeView::mouseMoveEvent(QMouseEvent *ev)
{
    bool accepted = false;
    if (m_d->is_dragging()) {
        m_d->handle_drag(ev->position());
        accepted = true;
    } else {
        update_cursor(ev);
    }

    m_d->drag_and_drop_start_pos = ev->pos();

    if (accepted) {
        ev->accept();
    } else {
        QTreeView::mouseMoveEvent(ev);
    }
}

void TimelineTreeView::mouseReleaseEvent(QMouseEvent *ev)
{
    QTreeView::mouseReleaseEvent(ev);
    m_d->timeline_drag_area = MousePositionArea::e_None;
    m_d->stop_dragging();
    update_cursor(ev);
}

void TimelineTreeView::mouseDoubleClickEvent(QMouseEvent *ev)
{
    auto column = static_cast<model::CompositionItemModel::Section>(columnAt(ev->pos().x()));
    if (column == model::CompositionItemModel::e_Features)
        return;

    bool accept = false;
    if (ev->button() == Qt::LeftButton) {
        QModelIndex index = QTreeView::indexAt(ev->pos());
        if (ev->button() == Qt::LeftButton && index.isValid()) {
            accept = m_d->handle_left_mouse_click(index, ev, visualRect(index), true);
        }
    }
    if (accept) {
        ev->accept();
        return;
    }
    QTreeView::mouseDoubleClickEvent(ev);
}

void TimelineTreeView::startDrag(Qt::DropActions supported_actions)
{
    if (m_d->is_dragging())
        return;

    QMimeData *data = model()->mimeData(selectedIndexes());
    if (!data)
        return;
    const QModelIndexList &indexes = selectedIndexes();
    if ((indexes.size() / 2) == 1) {
        QDrag *drag = new QDrag(this);
        QRect rect = visualRect(indexes.first());
        drag->setPixmap(grab(rect));
        drag->setMimeData(data);
        drag->setHotSpot(m_d->drag_and_drop_start_pos - rect.topLeft());
        drag->exec(supported_actions);
    }
}

void TimelineTreeView::dragEnterEvent(QDragEnterEvent *ev)
{
    // only two columns selects an index
    const QModelIndexList &indexes = selectedIndexes();
    if (!m_d->is_timeline_dragging && (indexes.size() / 2) == 1) {
        m_d->drag_and_drop_item
            = indexes.first().data(model::CompositionDocument::ItemRole).value<alive::TreeItem *>();
        ev->accept();
    } else {
        ev->acceptProposedAction();
        m_d->drag_and_drop_item = nullptr;
    }
}

void TimelineTreeView::dragLeaveEvent(QDragLeaveEvent *ev)
{
    QTreeView::dragLeaveEvent(ev);
}

void TimelineTreeView::dragMoveEvent(QDragMoveEvent *ev)
{
    QTreeView::dragMoveEvent(ev);
}

void TimelineTreeView::dropEvent(QDropEvent *ev)
{
    auto pos = ev->position();
    QModelIndex index = QTreeView::indexAt(QPoint(pos.x(), pos.y()));
    if (index.isValid()) {
        const QModelIndexList &indexes = selectedIndexes();
        if ((indexes.size() / 2) == 1) {
            TreeItem *item
                = index.data(model::CompositionDocument::ItemRole).value<alive::TreeItem *>();
            if (m_d->drag_and_drop_item && item) {
                m_d->doc->move_items(m_d->drag_and_drop_item, item, true);
            }
        }
        ev->acceptProposedAction();
    }
}

void TimelineTreeView::drawRow(QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    QTreeView::drawRow(painter, option, index);
    const auto *colors = alive::theme::ColorPalette::instance();
    painter->save();
    painter->setPen(colors->stroke_color());
    painter->drawRect(option.rect);
    painter->restore();
}

void TimelineTreeView::drawBranches(QPainter *painter,
                                    const QRect &rect,
                                    const QModelIndex &index) const
{
    painter->save();
    const auto *colors = alive::theme::ColorPalette::instance();
    const auto *sizes = alive::theme::TimelineSize::instance();
    painter->fillRect(rect, colors->secondary_background_color());
    if (model()->rowCount(index)) {
        painter->setPen(colors->primary_font_color());
        painter->translate(rect.right() - sizes->item_spacing() - sizes->icon_size().width() / 2,
                           rect.center().y());
        if (isExpanded(index)) {
            painter->drawPath(m_d->arrow_path);
        } else {
            painter->rotate(-90);
            painter->drawPath(m_d->arrow_path);
        }
    }
    painter->restore();
}

void TimelineTreeView::slot_selection_changed()
{
    auto [items, layers] = selected_items();
    model::Layer *selected_layer = nullptr;
    if (items.size()) {
        if (items.size() == 1 && layers.size() == 1) {
            selected_layer = layers.front();
        }
    } else {
        // nothing selected
    }
    emit layer_selected(selected_layer);
}

void TimelineTreeView::update_cursor(
    QMouseEvent *ev)
{
    QModelIndex index = QTreeView::indexAt(ev->pos());
    int x = ev->pos().x() - m_d->timeline_column_offset;
    if (x >= 0 && index.isValid()) {
        using namespace alive::model;
        bool is_layer = m_d->model->data(index, CompositionDocument::IsLayer).toBool();
        if (is_layer) {
            const LayerInOutPoints in_out_value = m_d->model
                                                      ->data(index,
                                                             CompositionDocument::LayerInOutRange)
                                                      .value<LayerInOutPoints>();
            m_d->update_cursor(
                m_d->find_overlap_area(x, in_out_value.in_point, in_out_value.out_point));
        } else {
            m_d->update_cursor(MousePositionArea::e_None);
        }
    } else {
        m_d->update_cursor(MousePositionArea::e_None);
    }
}

void TimelineTreeView::update_expression_editor_geometry()
{
    // set size of the editor
    QRect rect(m_d->range_selector->x(), 0, width() - m_d->range_selector->x(), height());
    m_d->expression_editor->setGeometry(rect);
    m_d->expression_editor->show();
    m_d->expression_editor->setFocus();
}

std::pair<std::map<alive::TreeItem *, int>, std::vector<model::Layer *> >
TimelineTreeView::selected_items()
{
    std::pair<std::map<alive::TreeItem *, int>, std::vector<model::Layer *> > result;

    auto &[items, layers] = result;

    auto indexes = selectedIndexes();
    for (const auto &index : indexes) {
        auto item = index.data(alive::model::CompositionDocument::ItemRole)
                        .value<alive::TreeItem *>();
        if (item) {
            items.emplace(item, index.row());
        }
    }

    for (const auto &item : items) {
        auto layer = item.first->to_layer();
        if (layer) {
            layers.emplace_back(layer);
        }
    }

    // sort layer by their visible order position
    std::sort(layers.begin(), layers.end(), [&](model::Layer *l1, model::Layer *l2) {
        return items[l1] < items[l2];
    });

#ifndef NDEBUG
    std::for_each(layers.begin(), layers.end(), [](const model::Layer *l) {
        qDebug() << l->name();
    });
#endif

    return result;
}

void init_timeline_globals()
{
    init_layer_menu();
}

// void TimelineTreeView::paintEvent(QPaintEvent *ev)
// {
//     QPainter p(this);
//     p.fillRect(this->rect(), Qt::red);
//     QTreeView::paintEvent(ev);
// }

} // namespace alive
