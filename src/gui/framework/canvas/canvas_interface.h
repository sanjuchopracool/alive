#ifndef CANVAS_WIDGET_H
#define CANVAS_WIDGET_H

#include <Corrade/Containers/Pointer.h>
#include <core/composition/canvasbuffer.h>
#include <core/alive_types/types.h>

namespace alive::model {
class CompositionDocument;
class CompositionNode;
} // namespace alive::model

namespace alive::canvas {
class CanvasInterface
{
public:
    using AliveDocument = alive::model::CompositionDocument;
    using AliveComposition = alive::model::CompositionNode;

    AliveDocument *document() const { return m_document; };

    float get_zoom_level() const { return m_zoom; }
    void zoom_in();
    void zoom_out();
    void zoom_to_fit();

    void resize_canvas(const alive::Size2Di &new_size);
    alive::Size2Di canvas_size() const { return m_canvas_size; }

private:
    void update_zoom();
    void update_buffer();

protected:
    virtual void on_buffer_size_update(){};

    void draw_on_canvas_buffer();

protected:
    explicit CanvasInterface(AliveDocument *doc);

    AliveDocument *m_document = nullptr;
    Corrade::Containers::Pointer<AliveComposition> m_composition;
    alive::CanvasBuffer m_buffer;

    float m_zoom = 1.00;
    alive::Point2D m_zoom_anchor;
    alive::Size2Di m_canvas_size = {};
};

} // namespace alive::canvas
#endif // CANVAS_WIDGET_H
