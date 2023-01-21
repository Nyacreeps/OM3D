#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <Texture.h>

#include <array>

namespace OM3D {

class Framebuffer : NonCopyable {
    public:
        template<size_t N>
        Framebuffer(Texture* depth, std::array<Texture*, N> colors) : Framebuffer(depth, colors.data(), colors.size()) {
        }


        Framebuffer();
        Framebuffer(Texture* depth);

        Framebuffer(Framebuffer&&) = default;
        Framebuffer& operator=(Framebuffer&&) = default;

        ~Framebuffer();

        void bind(bool clear = true, bool clearDepth = true) const;
        void blit(bool depth = false) const;
        void replace_texture(int number, const Texture* new_texture);

        const glm::uvec2& size() const;

    private:
        Framebuffer(Texture* depth, Texture** colors, size_t count);

        GLHandle _handle;
        glm::uvec2 _size = {};
};

}

#endif // FRAMEBUFFER_H
