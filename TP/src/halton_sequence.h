#ifndef HALTONSEQUENCE_H
#define HALTONSEQUENCE_H

#include <glm/vec2.hpp>

#include <array>

namespace OM3D {

float halton(unsigned int index, unsigned int base);

template <unsigned int N>
std::array<glm::vec2, N> pregenerate_halton_2_3() {
    std::array<glm::vec2, N> out;
    for (size_t i = 0; i < N; ++i) {
        out[i] = glm::vec2(halton(i + 1, 2), halton(i + 1, 3));
    }
    return out;
}

} // namespace OM3D
#endif /* ! HALTONSEQUENCE_H */