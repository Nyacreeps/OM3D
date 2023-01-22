#include "jitter.h"

#include <halton_sequence.h>

namespace OM3D {
static void convert_sub_pixel_jitter_to_world(glm::vec2& jitter, const glm::uvec2& window_size) {
    // formula: dxw = dxp * ((right - left)/ width)
    // - 0.5f to readjust the range of values from Halton's (0,1)x(0,1) to (-0.5,0.5)x(-0.5,0.5)
    constexpr float scale_factor = 1.0f;
    jitter.x = (jitter.x - 0.5f) * (1.0f / window_size.x) * scale_factor;
    jitter.y = (jitter.y - 0.5f) * (1.0f / window_size.y) * scale_factor;
}

JitterSequence init_jitter(const glm::uvec2& window_size) {
    auto jitter = pregenerate_halton_2_3<JITTER_POINTS>();
    for (size_t i = 0; i < JITTER_POINTS; ++i) {
        convert_sub_pixel_jitter_to_world(jitter[i], window_size);
    }
    return jitter;
}

} // namespace OM3D