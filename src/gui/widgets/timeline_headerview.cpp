#include "timeline_headerview.h"
#include "utility.h"
#include <gui/document/composition_document.h>
#include <gui/icon_manager.h>
#include <gui/settings/user_ui_setting.h>
#include <gui/framework/theme/color_palette.h>
#include <gui/theme/timeline_style.h>
#include <gui/widgets/edit_column_header.h>
#include <QLabel>
#include <QPainter>
#include <QSplitter>
#include <QStyleOption>

namespace alive {

class FeatureColumnHeader : public QWidget
{
public:
    FeatureColumnHeader(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        const auto *timeline_style = alive::theme::TimelineSize::instance();
        setFixedHeight(timeline_style->item_height());
        setFixedWidth(timeline_style->icon_size().width() * 3 + timeline_style->item_spacing() * 4);
    }

    void paintEvent(QPaintEvent *ev)
    {
        const IconManager *icon_mgr = IconManager::instance();
        QPainter painter(this);
        QRect rect(this->rect());
        const auto *colors = alive::theme::ColorPalette::instance();
        const auto *sizes = alive::theme::TimelineSize::instance();
        const int spacing = sizes->item_spacing();
        int icon_height = sizes->icon_size().height();
        QRect icon_rect(spacing,
                        (rect.height() - icon_height) / 2,
                        sizes->icon_size().width(),
                        icon_height);
        painter.fillRect(icon_rect, colors->primary_background_color());
        icon_mgr->get_icon(IconManager::e_Visible).paint(&painter, icon_rect);
        // lock state
        icon_rect.moveLeft(icon_rect.right() + spacing);
        painter.fillRect(icon_rect, colors->primary_background_color());
        icon_mgr->get_icon(IconManager::e_Lock).paint(&painter, icon_rect);
    }
};

QLabel *center_aligned_label(const QString &str)
{
    QLabel *label = new QLabel(str);
    label->setAlignment(Qt::AlignHCenter);
    return label;
}

QWidget *widget_for_column(int section, const alive::theme::TimelineSize *sizes)
{
    using namespace alive::model;
    switch (section) {
    case CompositionDocument::e_Features:
        return new FeatureColumnHeader();
    case CompositionDocument::e_Layer: {
        return center_aligned_label(QObject::tr("Source Name"));
    }
    case CompositionDocument::e_Edit:
        return new EditColumnHeader();
    case CompositionDocument::e_Tree: {
        QWidget *widget = new QWidget();
        widget->setMinimumWidth(2 * sizes->icon_size().width() + 3 * sizes->item_spacing());
        return widget;
    }
    default:
        return new QWidget();
    }
}

struct TimelineHeaderViewPrivateData
{
    QSplitter *splitter = nullptr;
    EditColumnHeader *edit_header = nullptr;
};

TimelineHeaderView::TimelineHeaderView(QWidget *parent)
    : QWidget(parent)
    , m_d(std::make_unique<TimelineHeaderViewPrivateData>())
{
    const auto *timeline_style = alive::theme::TimelineSize::instance();
    setFixedHeight(timeline_style->item_height());
    QVBoxLayout *layout = vbox_layout();
    m_d->splitter = new QSplitter(this);
    m_d->edit_header = new EditColumnHeader(this);
    const int count = static_cast<int>(alive::model::CompositionDocument::e_TimeLine);
    for (int i = 0; i < count; ++i) {
        if (i == alive::model::CompositionDocument::e_Edit) {
            m_d->splitter->addWidget(m_d->edit_header);
        } else {
            m_d->splitter->addWidget(widget_for_column(i, timeline_style));
        }

        m_d->splitter->setCollapsible(i, false);
    }

    QList<int> sizes;
    sizes.append(m_d->splitter->widget(0)->width());
    for (int i = 1; i < count; ++i) {
        sizes.append(100);
    }
    m_d->splitter->setSizes(sizes);
    m_d->splitter->setChildrenCollapsible(false);
    layout->addWidget(m_d->splitter);
    setLayout(layout);

    connect(m_d->splitter, &QSplitter::splitterMoved, this, &TimelineHeaderView::column_resized);
    connect(m_d->edit_header,
            &EditColumnHeader::show_matte_info_changed,
            this,
            [=, this](bool show, int delta) {
                QList<int> sizes;
                for (int i = 0; i < count; ++i) {
                    int width = m_d->splitter->widget(i)->width();
                    if (i == alive::model::CompositionDocument::e_Edit) {
                        width += delta;
                    }
                    sizes.append(width);
                }
                emit show_matte_info_changed(show, delta);
                m_d->splitter->setSizes(sizes);
                emit column_resized();
            });
}

TimelineHeaderView::~TimelineHeaderView()
{
    save_settings();
}

int TimelineHeaderView::no_of_columns() const
{
    return static_cast<int>(alive::model::CompositionDocument::e_TimeLine);
}

int TimelineHeaderView::width_for_column(int index) const
{
    using namespace alive::model;
    if (index < m_d->splitter->count()) {
        return m_d->splitter->widget(index)->width();
    }
    return 100;
}

int TimelineHeaderView::handle_width() const
{
    return m_d->splitter->handleWidth();
}

void TimelineHeaderView::paintEvent(QPaintEvent *ev)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    p.setPen(QPen(alive::theme::ColorPalette::instance()->stroke_color(), 1));
    QRectF rect(this->rect());
    p.drawRect(rect.adjusted(0.5, 0.5, -0.5, -0.5));
}

void TimelineHeaderView::restore_settings()
{
    {
        QSignalBlocker bl(m_d->edit_header);
        m_d->edit_header->set_show_matte_info(alive::UserSettings::instance()->show_matte_info());
    }

    QVariant state = alive::UserSettings::instance()->timeline_header_splitter_state();
    if (!state.isNull()) {
        m_d->splitter->restoreState(state.toByteArray());
    }
}

void TimelineHeaderView::show_matte_details(bool show)
{
    m_d->edit_header->set_show_matte_info(show);
}

void TimelineHeaderView::save_settings()
{
    alive::UserSettings::instance()->save_timeline_header_splitter_state(m_d->splitter->saveState());
    alive::UserSettings::instance()->set_show_matte_info(m_d->edit_header->show_matte_info());
}
} // namespace alive
