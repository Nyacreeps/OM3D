#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <utils.h>

#include <string_view>

static constexpr std::string_view shader_path = "../../shaders/";

class GLHandle : NonCopyable {
    public:
        GLHandle() = default;

        explicit GLHandle(u32 handle) : _handle(handle) {
        }

        GLHandle(GLHandle&& other) {
            swap(other);
        }

        GLHandle& operator=(GLHandle&& other) {
            swap(other);
            return *this;
        }

        void swap(GLHandle& other) {
            std::swap(_handle, other._handle);
        }

        u32 get() const {
            return _handle;
        }

        bool is_valid() const {
            return _handle;
        }

    private:
        u32 _handle = 0;
};

enum class BufferUsage {
    Attribute,
    Uniform,
    Index,
};

u32 buffer_usage_to_gl(BufferUsage usage);

void init_graphics();


#endif // GRAPHICS_H