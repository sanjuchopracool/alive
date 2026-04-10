#include "canvas_widget.h"
#include <core/composition/composition_node.h>
#include <core/model/composition.h>
#include <gui/document/composition_document.h>
#include <gui/settings/user_settings.h>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMimeData>

#include <core/constants/file_extension.h>
#include <gui/project/project_manager.h>
#include <QGuiApplication>
#include <QImage>
#include <QPainter>
#include <QResizeEvent>

#include <core/model/layers/image_layer.h>
#include <core/model/layers/precomposition_layer.h>
#include <gui/document/factories/editor_layer_factory.h>

namespace alive {
using namespace alive;

CanvasWidget::CanvasWidget(model::CompositionDocument *document, QWidget *parent)
    : QWidget(parent)
    , canvas::CanvasInterface(document)
{
    // support drop event
    setAcceptDrops(true);
    observe(m_composition.get());
    m_forced_update = true;
    gui::TimeLine *timeline = project::ProjectManager::instance()->document_timeline(document);
    m_current_frame = timeline->current_frame();
    project::ProjectManager::instance()->update_expression_engine(document, m_composition.get());
    Q_ASSERT(timeline);
    connect(timeline, &gui::TimeLine::play_frame_changed, this, [this](int frame) {
        slot_frame_changed(frame);
    });

    connect(m_document,
            &model::CompositionDocument::new_layer_added,
            this,
            [this](model::Layer *layer, int index) {
                m_composition->slot_new_layer_added(layer, index);
                m_update_node = true;
                slot_frame_changed(m_current_frame, false, true);
            });

    connect(m_document,
            &model::CompositionDocument::layer_index_changed,
            this,
            [this](model::Layer *layer, int index, bool is_matte) {
                m_composition->slot_layer_index_changed(layer, index, is_matte);
                m_update_node = true;
                slot_frame_changed(m_current_frame, true);
            });

    connect(m_document,
            &model::CompositionDocument::about_to_remove_layer,
            this,
            [this](model::Layer *layer) {
                m_composition->slot_about_to_remove_layer(layer);
                slot_frame_changed(m_current_frame, true);
            });

    connect(timeline, &gui::TimeLine::selected_frame_changed, this, [this](int frame) {
        m_update_node = true;
        slot_frame_changed(frame);
    });

    // shape related
    connect(m_document,
            &model::CompositionDocument::shape_added,
            this,
            [this](model::ShapeLayer *shape_layer,
                   model::Group *group,
                   model::ShapeItem *shape,
                   int index) {
                m_composition->slot_new_shape_added(shape_layer, group, shape, index);
                m_update_node = true;
                slot_frame_changed(m_current_frame, true);
            });

    connect(m_document,
            &model::CompositionDocument::about_to_remove_shape,
            this,
            [this](model::ShapeLayer *shape_layer, model::Group *group, model::ShapeItem *shape) {
                m_composition->slot_about_to_remove_shape(shape_layer, group, shape);
                slot_frame_changed(m_current_frame, true);
            });
    connect(m_document, &model::CompositionDocument::layer_moved, this, [this](int from, int to) {
        m_composition->slot_move_layer(from, to);
        m_update_node = true;
        slot_frame_changed(m_current_frame, true);
    });

    connect(m_document,
            &model::CompositionDocument::shapes_moved,
            this,
            [this](model::ShapeLayer *layer, model::Object *group, int from, int to) {
                m_composition->slot_move_shapes(layer, group, from, to);
                m_update_node = true;
                slot_frame_changed(m_current_frame, false, true);
            });

    connect(m_document,
            &model::CompositionDocument::applied_script,
            this,
            [this]() {
                m_update_node = true;
                slot_frame_changed(m_current_frame, false, true);
            });

    connect(m_document, &model::CompositionDocument::composition_settings_updated, this, [this]() {
        resize_window(this->size(), true);
    });

    // connect(this, &QQuickPaintedItem::windowChanged, this, [this](QQuickWindow *window) {
    //     if (window) {
    //         m_pixel_ratio = window->devicePixelRatio();
    //     }
    // });

    // checkerboard brush
    {
        constexpr int gird_size = 20;
        QPixmap pixmap(2 * gird_size, 2 * gird_size);
        pixmap.fill(Qt::white);
        QPainter p(&pixmap);
        p.fillRect(QRect(0, 0, gird_size, gird_size), Qt::lightGray);
        p.fillRect(QRect(gird_size, gird_size, gird_size, gird_size), Qt::lightGray);
        m_checkboard_brush.setTexture(pixmap);
    }
}

CanvasWidget::~CanvasWidget() {}

QUuid CanvasWidget::id() const
{
    if (m_document) {
        return m_document->id();
    }
    return {};
}

QString CanvasWidget::name() const
{
    if (m_document) {
        return m_document->name();
    }
    return {};
}

void CanvasWidget::paintEvent(QPaintEvent *ev)
{
    QPainter painter(this);
    if (m_composition && !m_buffer.empty()) {
        QImage::Format fm = QImage::Format_ARGB32;
        if (m_buffer.format() == CanvasBuffer::e_RGBA) {
            fm = QImage::Format_RGBA8888;
        } else if (m_buffer.format() == CanvasBuffer::e_BGRA) {
            fm = QImage::QImage::Format_ARGB32;
        } else {
            Q_ASSERT(false);
        }

        qreal dpr = devicePixelRatioF();
        QImage image(m_buffer.data(),
                     m_buffer.width(),
                     m_buffer.height(),
                     m_buffer.min_row_bytes(),
                     fm);

        painter.translate(width() / 2, height() / 2);
        auto *comp = m_document->composition();
        auto image_logical_width = comp->width() / dpr;
        auto image_logical_height = comp->height() / dpr;

        qreal scale = m_zoom;
        {
            qreal w = image_logical_width * scale;
            qreal h = image_logical_height * scale;
            QRect target_rect(-w / 2, -h / 2, w, h);
            // draw checker board
            // come back to original scale
            painter.fillRect(target_rect, m_checkboard_brush);
        }

        QRect target_rect(-image_logical_width / 2,
                          -image_logical_height / 2,
                          image_logical_width,
                          image_logical_height);
        painter.scale(m_zoom, m_zoom);
        painter.drawRect(target_rect);
        painter.drawImage(target_rect, image);
    }
}

void CanvasWidget::resizeEvent(QResizeEvent *ev)
{
    resize_window(ev->size(), true);
    QWidget::resizeEvent(ev);
}

void CanvasWidget::on_update(model::ISubject *)
{
    if (m_document->is_applying_script())
        return;

    if (!m_updating)
        slot_frame_changed(m_current_frame, false, true);
}

void CanvasWidget::slot_frame_changed(int frame, bool only_render, bool update_and_render)
{
    if (m_current_frame != frame) {
        emit current_frame_changed();
    }
    m_current_frame = frame;
    //    static int counter = 1;
    FrameTimeType t = static_cast<FrameTimeType>(frame);
    //    qDebug() << t << counter++;
    if (!m_updating) {
        if (m_composition) {
            m_updating = true;
            //        AutoProfiler p("U");
            if (update_and_render) {
                m_composition->update(t, m_forced_update);
                draw_on_canvas_buffer();
                QWidget::update();
            } else if (only_render) {
                draw_on_canvas_buffer();
                QWidget::update();
            } else if (m_composition->update(t, m_forced_update)) {
                draw_on_canvas_buffer();
                QWidget::update();
            }

            m_forced_update = false;
            m_updating = false;

            if (m_update_node) {
                m_document->container_nodes_updated();
                m_update_node = false;
            }
        }
    }
}

void CanvasWidget::on_buffer_size_update()
{
    slot_frame_changed(m_current_frame, true);
}

void CanvasWidget::resize_window(const QSize &size, bool forced)
{
    if (forced) {
        qreal dpr = devicePixelRatioF();
        resize_canvas({static_cast<int>(size.width() * dpr), static_cast<int>(size.height() * dpr)});
        slot_frame_changed(m_current_frame, false, forced);
    }
}

void CanvasWidget::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void CanvasWidget::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void CanvasWidget::dragLeaveEvent(QDragLeaveEvent *event)
{
    event->accept();
}

void CanvasWidget::dropEvent(QDropEvent *event)
{
    bool accepted = false;
    auto data = event->mimeData();
    if (data && data->hasUrls()) {
        auto urls = data->urls();
        QStringList supported_image_formats{"png", "jpeg", "jpg"};

        std::vector<model::Layer *> layers;
        for (const auto &url : urls) {
            QFileInfo info(url.toLocalFile());
            QString ext = info.completeSuffix().toLower();
            if (supported_image_formats.contains(ext)) {
                model::ImageLayer *layer = static_cast<model::ImageLayer *>(
                    m_document->create_new_layer(model::LayerType::e_Image).first);
                if (layer) {
                    layer->set_path(url.toLocalFile().toStdString());
                    layers.emplace_back(layer);
                    accepted = true;
                }
            } else if (ext == file_extensions::k_alive || ext == file_extensions::k_json) {
                model::PrecompositionLayer *layer = static_cast<model::PrecompositionLayer *>(
                    m_document->create_new_layer(model::LayerType::e_Precomp).first);
                if (layer) {
                    layer->set_precomp_path(url.toLocalFile().toStdString());
                    const auto &size = layer->precomp_size();
                    layer->transform().set_anchor(
                        {static_cast<float>(size.x() / 2.0), static_cast<float>(size.y() / 2.0), 0});
                    layer->sync_in_out_with_precomp();
                    layers.emplace_back(layer);
                    accepted = true;
                }
            }
        }
        if (!layers.empty()) {
            m_document->add_new_layers_command(layers);
        }
    }

    if (accepted) {
        event->acceptProposedAction();
    } else {
        QWidget::dropEvent(event);
    }
}

//void CanvasItem::focusInEvent(QFocusEvent *ev)
//{
//    ev->accept();
//}

//void CanvasItem::focusOutEvent(QFocusEvent *ev)
//{
//    ev->accept();
//}
} // namespace alive::gui
