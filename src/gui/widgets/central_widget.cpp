#include "central_widget.h"
#include <core/model/layers/text_layer.h>
#include <core/model/text_primitives/text.h>
#include <gui/document/composition_document.h>
#include <gui/framework/widgets/toolbar_property_widget.h>
#include <gui/project/global_selection_manager.h>
#include <gui/project/project_manager.h>
#include <gui/settings/user_ui_setting.h>
#include <gui/widgets/canvas_tab_widget.h>
#include <gui/widgets/timeline_tab_widget.h>
#include <gui/widgets/utility.h>
#include <QSplitter>

namespace alive {

struct CentralWidgetPrivateData
{
    CanvasTabWidget *canvas_area = nullptr;
    TimelineTabWidget *timeline_area = nullptr;
    QSplitter *main_splitter = nullptr;
    QSplitter *top_horizontal_splitter = nullptr;

    ToolBarPropertyWidget *right_toolbar_widget = nullptr;
    GlobalSelectionManager *selection_manager = nullptr;
};

CentralWidget::CentralWidget(QWidget *parent)
    : QWidget{parent}
    , m_d(std::make_unique<CentralWidgetPrivateData>())
{
    m_d->main_splitter = new QSplitter(Qt::Vertical, this);
    m_d->canvas_area = new CanvasTabWidget(this);
    m_d->timeline_area = new TimelineTabWidget(this);

    // text
    QWidget *temp = new QWidget(this);

    // toolbar
    m_d->right_toolbar_widget = new alive::ToolBarPropertyWidget(this);
    m_d->selection_manager = new GlobalSelectionManager(m_d->right_toolbar_widget, this);

    m_d->top_horizontal_splitter = new QSplitter(Qt::Horizontal, temp);
    m_d->top_horizontal_splitter->addWidget(m_d->canvas_area);
    m_d->top_horizontal_splitter->addWidget(m_d->right_toolbar_widget);

    // splitter related
    {
        m_d->top_horizontal_splitter->setCollapsible(m_d->top_horizontal_splitter->indexOf(
                                                         m_d->right_toolbar_widget),
                                                     false);

        connect(m_d->right_toolbar_widget,
                &ToolBarPropertyWidget::toolbar_visibility_changed,
                this,
                [this](int size) {
                    auto sizes = m_d->top_horizontal_splitter->sizes();
                    if (sizes.size() >= 2) {
                        int diff = sizes.last() - size;
                        sizes.last() = size;
                        int second_last = sizes.size() - 2;
                        sizes[second_last] += diff;
                    }
                    m_d->top_horizontal_splitter->setSizes(sizes);
                });

        connect(m_d->top_horizontal_splitter, &QSplitter::splitterMoved, this, [this]() {
            m_d->right_toolbar_widget->update_expanded_width();
        });
    }

    auto *top_horizontal_layout = hbox_layout(1);
    top_horizontal_layout->addWidget(m_d->top_horizontal_splitter);
    temp->setLayout(top_horizontal_layout);

    m_d->main_splitter->addWidget(temp);
    m_d->main_splitter->addWidget(m_d->timeline_area);

    auto *layout = hbox_layout(1);
    layout->addWidget(m_d->main_splitter);
    setLayout(layout);

    auto *prj_mgr = alive::project::ProjectManager::instance();
    connect(prj_mgr,
            &alive::project::ProjectManager::document_opened,
            this,
            [this](alive::model::CompositionDocument *doc) {
                m_d->canvas_area->open_tab_for_doc(doc);
                m_d->timeline_area->open_tab_for_doc(doc);
            });

    connect(prj_mgr, &alive::project::ProjectManager::current_doc_changed, this, [=, this]() {
        alive::model::CompositionDocument *doc = prj_mgr->current_document();
        m_d->canvas_area->select_tab_for_doc(doc);
        m_d->timeline_area->select_tab_for_doc(doc);
    });

    connect(prj_mgr,
            &alive::project::ProjectManager::about_to_close_document,
            this,
            [=, this](alive::model::CompositionDocument *doc) {
                m_d->canvas_area->close_tab_for_doc(doc);
                m_d->timeline_area->close_tab_for_doc(doc);
            });

    connect(m_d->canvas_area,
            &CanvasTabWidget::selected_tab_for_doc,
            this,
            [=, this](alive::model::CompositionDocument *doc) {
                prj_mgr->select_current_doc_with_id(doc->id());
            });

    connect(m_d->timeline_area,
            &TimelineTabWidget::selected_tab_for_doc,
            this,
            [=, this](alive::model::CompositionDocument *doc) {
                prj_mgr->select_current_doc_with_id(doc->id());
            });

    connect(m_d->timeline_area,
            &TimelineTabWidget::layer_selected,
            m_d->selection_manager,
            &GlobalSelectionManager::on_layer_selected);
}

CentralWidget::~CentralWidget()
{
}

CanvasTabWidget *CentralWidget::canvas_tab_widget()
{
    return m_d->canvas_area;
}

void alive::CentralWidget::restore_settings()
{
    QVariant main_splitter_state = alive::UserSettings::instance()->main_window_splitter_state();
    if (!main_splitter_state.isNull()) {
        m_d->main_splitter->restoreState(main_splitter_state.toByteArray());
    }

    m_d->right_toolbar_widget->restore_settings();
    QVariant top_horizontal_splitter_state = alive::UserSettings::instance()
                                                 ->top_horizontal_splitter_state();
    if (!top_horizontal_splitter_state.isNull()) {
        m_d->top_horizontal_splitter->restoreState(top_horizontal_splitter_state.toByteArray());
    }
}

void CentralWidget::save_settings() const
{
    alive::UserSettings::instance()->save_main_window_splitter_state(m_d->main_splitter->saveState());
    m_d->right_toolbar_widget->save_settings();
    alive::UserSettings::instance()->save_top_horizontal_splitter_state(
        m_d->top_horizontal_splitter->saveState());
}

} // namespace alive
