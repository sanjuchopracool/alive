#include "canvas_interface.h"
#include <core/composition/composition_node.h>
#include <core/model/composition.h>
#include <gui/document/composition_document.h>

namespace alive::canvas {

constexpr float MIN_CANVAS_ZOOM = 0.1f;
constexpr float MAX_CANVAS_ZOOM = 1.0f;
void CanvasInterface::zoom_in()
{
    m_zoom *= 1.2;
    update_zoom();
}

void CanvasInterface::zoom_out()
{
    float zoom = m_zoom / 1.2;
    zoom = std::max(MIN_CANVAS_ZOOM, zoom);
    if (zoom != m_zoom) {
        m_zoom = zoom;
        update_zoom();
    }
}

void CanvasInterface::zoom_to_fit()
{
    auto comp = m_document->composition();
    using namespace alive;
    Vec1D width = comp->width();
    Vec1D height = comp->height();

    auto target_width = m_canvas_size.x();
    auto target_height = m_canvas_size.y();
    Vec1D h = (height / width) * target_width;
    Vec1D w = 0;
    Vec1D scale;
    if (h > target_height) {
        h = target_height;
        scale = h / height;
        w = (width / height) * target_height;
    } else {
        w = target_width;
        scale = w / width;
    }

    m_zoom = scale;
    if (m_zoom < MIN_CANVAS_ZOOM) {
        m_zoom = MIN_CANVAS_ZOOM;
    }
    update_zoom();
}

void CanvasInterface::resize_canvas(const alive::Size2Di &new_size)
{
    m_canvas_size = new_size;
    update_buffer();
}

void CanvasInterface::update_zoom()
{
    update_buffer();
    on_buffer_size_update();
}

void CanvasInterface::update_buffer()
{
    float zoom = m_zoom;
    zoom = std::clamp(zoom, MIN_CANVAS_ZOOM, MAX_CANVAS_ZOOM);

    auto comp = m_document->composition();
    m_buffer.resize(comp->width() * zoom, comp->height() * zoom);
}

void CanvasInterface::draw_on_canvas_buffer()
{
    auto scale = std::min(MAX_CANVAS_ZOOM, m_zoom);
    m_buffer.draw(m_composition.get(),
                  alive::Matrix3D::scaling({scale, scale}),
                  m_document->composition()->show_background_color());
}

CanvasInterface::CanvasInterface(AliveDocument *doc)
{
    m_document = doc;
    m_composition = Corrade::Containers::pointer<AliveComposition>(m_document->composition(), true);
    update_buffer();
}

} // namespace alive::canvas
