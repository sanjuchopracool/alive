#include "timeline_widget.h"
#include "timeline_header_widget.h"
#include "timeline_headerview.h"
#include "timeline_playhead.h"
#include "timeline_range_selector.h"
#include "timeline_treeview.h"
#include "utility.h"
#include <gui/action_contexts.h>
#include <gui/document/composition_document.h>
#include <gui/document/composition_document_proxy_model.h>
#include <gui/framework/managers/action_manager.h>
#include <gui/project/project_manager.h>
#include <gui/settings/user_ui_setting.h>
#include <gui/theme/timeline_style.h>
#include <QApplication>
#include <QMenu>
#include <QPainter>
#include <QResizeEvent>
#include <QScrollBar>
#include <QSplitter>
#include <QStyle>
#include <QStyleOption>

namespace alive {

struct TimelineWidgetData
{
    TimelineWidgetData(alive::model::CompositionDocument *doc)
        : document(doc)
    {}
    alive::model::CompositionDocument *document = nullptr;
    TimelineRangeSelector *range_selector = nullptr;
    TimelineTreeView *tree_view = nullptr;
    TimelineHeaderWidget *header_widget = nullptr;
    TimelineHeaderView *header_view = nullptr;
    TimelinePlayHead *selected_frame_indicator = nullptr;
    TimelinePlayHead *play_frame_indicator = nullptr;
    QSplitter *top_horizontal_splitter = nullptr;
    bool init_done = false;
};

TimelineWidget::TimelineWidget(QWidget *parent, alive::model::CompositionDocument *doc)
    : QWidget(parent)
    , m_d(std::make_unique<TimelineWidgetData>(doc))
{
    m_d->range_selector = new TimelineRangeSelector(
        alive::project::ProjectManager::instance()->document_timeline(doc));
    m_d->range_selector->setMinimumWidth(200);
    const auto *helper = m_d->range_selector->timeline_helper();

    m_d->tree_view = new TimelineTreeView(doc, m_d->range_selector, this);
    const int tree_column = static_cast<int>(alive::model::CompositionDocument::e_Tree);
    m_d->tree_view->setTreePosition(tree_column);

    {
        m_d->header_widget = new TimelineHeaderWidget(this);
        m_d->header_view = m_d->header_widget->header_view();
        connect(m_d->header_widget,
                &TimelineHeaderWidget::filter_changed,
                this,
                [this](QString filter) { m_d->document->proxy_model()->filter(filter); });
    }

    QVBoxLayout *layout = vbox_layout();
    // QVBoxLayout *header_layout = vbox_layout();
    // header_layout->addWidget(m_d->header_view);

    QWidget *space = new QWidget(this);
    space->setFixedWidth(qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent));

    m_d->top_horizontal_splitter = new QSplitter(this);
    m_d->top_horizontal_splitter->addWidget(m_d->header_widget);
    m_d->top_horizontal_splitter->addWidget(m_d->range_selector);
    m_d->top_horizontal_splitter->addWidget(space);
    m_d->top_horizontal_splitter->setCollapsible(0, false);
    m_d->top_horizontal_splitter->setCollapsible(1, false);
    m_d->top_horizontal_splitter->setCollapsible(2, false);

    layout->addWidget(m_d->top_horizontal_splitter);
    layout->addWidget(m_d->tree_view);
    setLayout(layout);
    m_d->selected_frame_indicator = new TimelinePlayHead(this);
    m_d->play_frame_indicator = new TimelinePlayHead(this, TimelinePlayHead::e_Play);

    connect(helper,
            &alive::gui::TimelineHelper::play_frame_changed,
            this,
            &TimelineWidget::update_play_indicator);

    connect(helper, &alive::gui::TimelineHelper::range_changed, this, [this] {
        m_d->tree_view->viewport()->update();
    });

    connect(m_d->tree_view,
            &TimelineTreeView::layer_selected,
            this,
            &TimelineWidget::layer_selected);

    connect(helper,
            &alive::gui::TimelineHelper::selected_frame_position_changed,
            this,
            &TimelineWidget::update_selected_playhead);

    connect(helper,
            &alive::gui::TimelineHelper::runstate_changed,
            this,
            &TimelineWidget::update_play_indicator);

    connect(helper, &alive::gui::TimelineHelper::selected_frame_position_changed, this, [this]() {
        m_d->tree_view->viewport()->update();
    });

    connect(m_d->header_view,
            &TimelineHeaderView::column_resized,
            this,
            &TimelineWidget::update_element_size);

    connect(m_d->top_horizontal_splitter, &QSplitter::splitterMoved, this, [this] {
        update_element_size();
    });

    connect(m_d->header_view,
            &TimelineHeaderView::show_matte_info_changed,
            this,
            [this](bool show, int delta) {
                m_d->document->set_show_matte_details(show);
                QList<int> sizes;
                sizes.append(m_d->top_horizontal_splitter->widget(0)->width() + delta);
                sizes.append(m_d->top_horizontal_splitter->widget(1)->width() - delta);
                sizes.append(m_d->top_horizontal_splitter->widget(2)->width());
                m_d->top_horizontal_splitter->setSizes(sizes);
            });

    restore_settings();

    for (auto *action : ActionManager::instance()->get_panel_context_actions(
             action_context_ids::k_panel_timeline_context)) {
        addAction(action);
    }
}

TimelineWidget::~TimelineWidget()
{
    save_settings();
}

alive::model::CompositionDocument *TimelineWidget::document() const
{
    return m_d->document;
}

void TimelineWidget::resizeEvent(QResizeEvent *ev)
{
    QWidget::resizeEvent(ev);
    // qDebug() << "Range Selector Width = " << m_d->range_selector->width();
    update_element_size();
}

void TimelineWidget::showEvent(QShowEvent *ev)
{
    QWidget::showEvent(ev);
    if (!m_d->init_done) {
        update_column_size();
        m_d->init_done = true;
    }
}

void TimelineWidget::update_play_indicator()
{
    const auto *helper = m_d->range_selector->timeline_helper();
    const int y = m_d->range_selector->height();
    int x = helper->position_for_frame(helper->play_frame());
    if (helper->is_running()) {
        if (x > 0) {
            x += m_d->range_selector->geometry().x();
            m_d->play_frame_indicator->setGeometry(x, y, 1, static_cast<qreal>(height()));
            m_d->play_frame_indicator->show();
        } else {
            m_d->play_frame_indicator->hide();
        }
    } else {
        m_d->play_frame_indicator->hide();
    }
}

void TimelineWidget::update_selected_playhead()
{
    const auto *sizes = alive::theme::TimelineSize::instance();
    const auto *helper = m_d->range_selector->timeline_helper();
    if (helper->selected_frame_position_x() > 0) {
        m_d->selected_frame_indicator->show();
        const qreal x = m_d->range_selector->geometry().x() + helper->selected_frame_position_x();
        m_d->selected_frame_indicator->setGeometry(x - sizes->playhead_size().width() / 2,
                                                   0,
                                                   sizes->playhead_size().width(),
                                                   static_cast<qreal>(height()));
    } else {
        m_d->selected_frame_indicator->hide();
    }
}

void TimelineWidget::update_column_size()
{
    const int count = m_d->header_view->no_of_columns();
    const int handle_width = m_d->header_view->handle_width();
    int layer_column_width = 0;
    for (int i = 0; i < count; ++i) {
        int offset = i ? handle_width : 0;
        // for last column add two
        int width = m_d->header_view->width_for_column(i);

        if (i == count - 1) {
            offset *= 2;
        }

        width += offset;
        if (static_cast<int>(alive::model::CompositionDocument::e_Layer) == i) {
            layer_column_width = width;
        }
        m_d->tree_view->setColumnWidth(i, width);
    }
    m_d->tree_view->adjust_after_column_resize(layer_column_width);
}

void TimelineWidget::restore_settings()
{
    QVariant state = alive::UserSettings::instance()->timeline_main_splitter_state();
    if (!state.isNull()) {
        m_d->top_horizontal_splitter->restoreState(state.toByteArray());
    }
    m_d->header_view->restore_settings();
    m_d->document->set_show_matte_details(alive::UserSettings::instance()->show_matte_info());
}

void TimelineWidget::save_settings()
{
    alive::UserSettings::instance()->save_timeline_main_splitter_state(
        m_d->top_horizontal_splitter->saveState());
}

void TimelineWidget::update_element_size()
{
    update_column_size();
    update_selected_playhead();
    update_play_indicator();
}

} // namespace alive
