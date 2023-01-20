#ifndef MATERIAL_H
#define MATERIAL_H

#include <Program.h>
#include <Texture.h>

#include <memory>
#include <vector>

namespace OM3D {

enum class BlendMode {
    None,
    Alpha,
    Additive,
};

enum class DepthTestMode { Standard, Reversed, Equal, None };

enum class DepthMaskMode { True, False };

enum class RenderMode {
    INSTANCED,
    NON_INSTANCED,
    OCC_DEBUG
};

class Material {
public:
    Material();

    void set_program(std::shared_ptr<Program> prog);
    void set_blend_mode(BlendMode blend);
    void set_depth_test_mode(DepthTestMode depth);
    void set_depth_mask_mode(DepthMaskMode mask);
    void set_texture(u32 slot, std::shared_ptr<Texture> tex);

    template <typename... Args>
    void set_uniform(RenderMode render, Args&&... args) {
        switch (render) {
            case RenderMode::INSTANCED:
                _program->set_uniform(FWD(args)...);
                break;
            case RenderMode::NON_INSTANCED:
                _program2->set_uniform(FWD(args)...);
                break;
            case RenderMode::OCC_DEBUG:
                _program3->set_uniform(FWD(args)...);
        }
    }

    void bind(RenderMode render) const;

    static std::shared_ptr<Material> empty_material();
    static Material textured_material();
    static Material textured_normal_mapped_material();

private:
    std::shared_ptr<Program> _program;
    std::shared_ptr<Program> _program2;
    std::shared_ptr<Program> _program3;
    std::vector<std::pair<u32, std::shared_ptr<Texture>>> _textures;

    BlendMode _blend_mode = BlendMode::None;
    DepthTestMode _depth_test_mode = DepthTestMode::Standard;
    DepthMaskMode _depth_mask_mode = DepthMaskMode::True;

public:
    int uid;
};

} // namespace OM3D

#endif // MATERIAL_H
