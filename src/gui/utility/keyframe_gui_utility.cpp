#include "keyframe_gui_utility.h"
#include <QPainterPath>

namespace alive::keyFrameGuiUtility {
constexpr int k_frame_h_size = 7;

QPainterPath m_no_easing;
QPainterPath m_both_easing;
QPainterPath m_out_easing;
QPainterPath m_in_easing;
bool is_generated = false;

using namespace alive;
const QPainterPath &path_for_keyframe(const model::KeyFrame *keyframe)
{
    switch (keyframe->easing_type()) {
    case model::KeyFrame::e_Both:
        return m_both_easing;
    case model::KeyFrame::e_Out:
        return m_out_easing;
    case model::KeyFrame::e_In:
        return m_in_easing;
    default:
        return m_no_easing;
        break;
    }
}

void generate_keyframe_path()
{
    if (!is_generated) {
        m_no_easing.moveTo(k_frame_h_size, 0);
        m_no_easing.lineTo(0, k_frame_h_size);
        m_no_easing.lineTo(-k_frame_h_size, 0);
        m_no_easing.lineTo(0, -k_frame_h_size);
        m_no_easing.closeSubpath();

        QRectF bbox(0, 0, 2 * k_frame_h_size, 2 * k_frame_h_size);
        bbox.moveCenter(QPointF(1.5 * k_frame_h_size, 0));
        m_out_easing.moveTo(1.5 * k_frame_h_size, k_frame_h_size);
        m_out_easing.arcTo(bbox, 270, -180);
        m_out_easing.lineTo(0, -k_frame_h_size);
        m_out_easing.lineTo(-k_frame_h_size, 0);
        m_out_easing.lineTo(0, k_frame_h_size);
        m_out_easing.closeSubpath();

        bbox.moveCenter(QPointF(1.5 * k_frame_h_size, 0));
        m_both_easing.moveTo(1.5 * k_frame_h_size, k_frame_h_size);
        m_both_easing.arcTo(bbox, 270, -180);
        m_both_easing.lineTo(-1.5 * -k_frame_h_size, -k_frame_h_size);
        bbox.moveCenter(QPointF(-1.5 * k_frame_h_size, 0));
        m_both_easing.arcTo(bbox, 80, -180);
        m_both_easing.closeSubpath();

        m_in_easing.moveTo(k_frame_h_size, 0);
        m_in_easing.lineTo(0, -k_frame_h_size);
        m_in_easing.lineTo(-1.5 * k_frame_h_size, -k_frame_h_size);
        bbox.moveCenter(QPointF(-1.5 * k_frame_h_size, 0));
        m_in_easing.arcTo(bbox, 80, -180);
        m_in_easing.lineTo(0, k_frame_h_size);
        m_in_easing.closeSubpath();

        is_generated = true;
    }
}
} // namespace alive::keyFrameGuiUtility
