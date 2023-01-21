#include "jitter.h"

#include <halton_sequence.h>

namespace OM3D {
static void convert_sub_pixel_jitter_to_world(glm::vec2& jitter, const glm::uvec2& window_size) {
    // formula: dxw = dxp * ((right - left)/ width)
    jitter.x *= (1.0f / window_size.x);
    jitter.y *= (1.0f / window_size.y);
}

JitterSequence init_jitter(const glm::uvec2& window_size) {
    auto jitter = pregenerate_halton_2_3<JITTER_POINTS>();
    for (size_t i = 0; i < 16; ++i) {
        convert_sub_pixel_jitter_to_world(jitter[i], window_size);
    }
    return jitter;
}

} // namespace OM3D