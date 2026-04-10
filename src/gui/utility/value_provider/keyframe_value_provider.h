#ifndef KEYFRAMEVALUEPROVIDER_H
#define KEYFRAMEVALUEPROVIDER_H

#include <model/keyframes/keyframe.h>
#include <QDebug>

namespace inae::model {

template<typename T>
class KeyFrameValueProvider
{
public:
    using valueType = T;
    using KeyFrameType = KeyFrame<T>;

    KeyFrameValueProvider(const KeyFrames<T> &frames)
        : m_keyframes(frames)
    {}

    const T &value() const { return m_value; }

    bool has_update(FrameType time) const
    {
        if (m_last_updated_frame == -1)
            return true;

        if (m_keyframes.size() == 1)
            return false;

        if (is_valid_index(m_trailing_index) and (not is_valid_index(m_leading_index))) {
            if (time < m_keyframes[m_trailing_index].m_time)
                return false;
        }

        if (is_valid_index(m_leading_index) and (not is_valid_index(m_trailing_index))) {
            if (m_keyframes[m_leading_index].m_time < time)
                return false;
        }

        if (is_valid_index(m_leading_index) and is_valid_index(m_trailing_index)
            and m_keyframes[m_leading_index].m_is_hold) {
            if ((m_keyframes[m_leading_index].m_time < time)
                and (time < m_keyframes[m_trailing_index].m_time))
                return true;
        }

        return true;
    }

    void update(FrameType time)
    {
        update_indices(time);
        if (is_valid_index(m_leading_index) and is_valid_index(m_trailing_index)) {
            // interpolate
            double progress
                = m_keyframes[m_leading_index].interpolated_progress(m_keyframes[m_trailing_index],
                                                                     time);
            m_value = m_keyframes[m_leading_index].interpolate_to(m_keyframes[m_trailing_index],
                                                                  progress);
        } else if (is_valid_index(m_leading_index)) {
            m_value = m_keyframes[m_leading_index].m_value;
        } else if (is_valid_index(m_trailing_index)) {
            m_value = m_keyframes[m_trailing_index].m_value;
        } else {
            m_value = m_keyframes[0].m_value;
        }
    }

    bool is_static() const { return (m_keyframes.size() == 1); }

private:
    void update_indices(FrameType time)
    {
        m_last_updated_frame = time;

        if (m_keyframes.size() == 1) {
            m_leading_index = 0;
            m_trailing_index = -1;
            return;
        }

        if (-1 == m_leading_index && -1 == m_trailing_index) {
            if (time < m_keyframes[0].m_time)
                m_trailing_index = 0;
            else {
                m_leading_index = 0;
                m_trailing_index = 1;
            }
        }

        // move ahead
        if ((-1 != m_trailing_index) and (m_keyframes[m_trailing_index].m_time < time)) {
            while (1) {
                m_leading_index = m_trailing_index;
                m_trailing_index = valid_index(m_leading_index + 1);

                if (-1 == m_trailing_index)
                    break;

                if (time < m_keyframes[m_trailing_index].m_time)
                    break;
            }
        }

        // move back
        else if ((-1 != m_leading_index) and (time < m_keyframes[m_leading_index].m_time)) {
            while (1) {
                m_trailing_index = m_leading_index;
                m_leading_index = valid_index(m_leading_index - 1);

                if (-1 == m_leading_index)
                    break;

                if (m_keyframes[m_leading_index].m_time < time)
                    break;
            }
        }
    }

    int valid_index(int index) const
    {
        return (m_keyframes.size() > index) ? (index >= 0) ? index : -1 : -1;
    }

    bool is_valid_index(int index) const { return (index >= 0) and index < m_keyframes.size(); }

private:
    T m_value;

    const std::vector<KeyFrameType> &m_keyframes;

    FrameType m_last_updated_frame = -1;
    int m_leading_index = -1;
    int m_trailing_index = -1;
};
} // namespace inae::model

#endif // KEYFRAMEVALUEPROVIDER_H
