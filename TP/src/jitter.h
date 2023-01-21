#ifndef JITTER_h
#define JITTER_h

#include <glm/vec2.hpp>
#include <array>

namespace OM3D {

constexpr size_t JITTER_POINTS = 16;
using JitterSequence = std::array<glm::vec2, JITTER_POINTS>;

/**
 * \brief Initialises an array of subpixel jitter displacements in world space
 * 
 * \param window_size Size of the window/viewport in pixels
 * \return std::array<glm::vec2, JITTER_POINTS>
 *      (usually JITTER_POINTS = 16)
 */
JitterSequence init_jitter(const glm::uvec2& window_size);

} // namespace OM3D

#endif /* ! JITTER_h */