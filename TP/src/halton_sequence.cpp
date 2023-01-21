#include "halton_sequence.h"

float halton(unsigned int index, unsigned int base) {
    float f = 1.0f;
    float r = 0.0f;

    while (index > 0) {
        f /= static_cast<float>(base);
        r += f * static_cast<float>(index % base);
        index /= base;
    }

    return r;
}