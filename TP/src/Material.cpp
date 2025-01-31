#include "Material.h"

#include <glad/glad.h>

#include <algorithm>

namespace OM3D {

Material::Material() {
    static int i = 0;
    uid = ++i;
}

void Material::set_program(std::shared_ptr<Program> prog) {
    _program = std::move(prog);
}

void Material::set_blend_mode(BlendMode blend) {
    _blend_mode = blend;
}

void Material::set_depth_test_mode(DepthTestMode depth) {
    _depth_test_mode = depth;
}

void Material::set_depth_mask_mode(DepthMaskMode mask) {
    _depth_mask_mode = mask;
}

void Material::set_texture(u32 slot, std::shared_ptr<Texture> tex) {
    if (const auto it = std::find_if(_textures.begin(), _textures.end(),
                                     [&](const auto& t) { return t.second == tex; });
        it != _textures.end()) {
        it->second = std::move(tex);
    } else {
        _textures.emplace_back(slot, std::move(tex));
    }
}

void Material::bind(RenderMode render) const {
    switch (_blend_mode) {
        case BlendMode::None:
            glDisable(GL_BLEND);
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            break;

        case BlendMode::Alpha:
            glEnable(GL_BLEND);
            glDisable(GL_CULL_FACE);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            break;

        case BlendMode::Additive:
            glEnable(GL_BLEND);
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
            glBlendEquation(GL_FUNC_ADD);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            break;
    }

    if (render == RenderMode::OCC_DEBUG) goto nodepthtest;
    switch (_depth_test_mode) {
        case DepthTestMode::None:
        nodepthtest:
            glDisable(GL_DEPTH_TEST);
            break;

        case DepthTestMode::Equal:
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_EQUAL);
            break;

        case DepthTestMode::Standard:
            glEnable(GL_DEPTH_TEST);
            // We are using reverse-Z
            glDepthFunc(GL_GEQUAL);
            break;

        case DepthTestMode::Reversed:
            glEnable(GL_DEPTH_TEST);
            // We are using reverse-Z
            glDepthFunc(GL_LEQUAL);
            break;
    }

    switch (_depth_mask_mode) {
        case DepthMaskMode::True:
            glDepthMask(GL_TRUE);
            break;
        case DepthMaskMode::False:
            glDepthMask(GL_FALSE);
            break;
    }

    for (const auto& texture : _textures) {
        texture.second->bind(texture.first);
    }
    switch (render) {
        case RenderMode::INSTANCED:
            _program->bind();
            break;
        case RenderMode::NON_INSTANCED:
            _program2->bind();
            break;
        case RenderMode::OCC_DEBUG:
            _program3->bind();
            break;
    }
}

std::shared_ptr<Material> Material::empty_material() {
    static std::weak_ptr<Material> weak_material;
    auto material = weak_material.lock();
    if (!material) {
        material = std::make_shared<Material>();
        material->_program = Program::from_files("prepass.frag", "prepass_instanced.vert");
        material->_program2 = Program::from_files("prepass.frag", "basic.vert");
        material->_program3 = Program::from_files("prepass_debugocc.frag", "basic.vert");
        weak_material = material;
    }
    return material;
}

Material Material::textured_material() {
    Material material;
    material._program = Program::from_files("prepass.frag", "prepass_instanced.vert", {"TEXTURED"});
    material._program2 = Program::from_files("prepass.frag", "basic.vert", {"TEXTURED"});
    material._program3 = Program::from_files("prepass_debugocc.frag", "basic.vert", {"TEXTURED"});
    return material;
}

Material Material::textured_normal_mapped_material() {
    Material material;
    material._program =
        Program::from_files("prepass.frag", "prepass_instanced.vert",
                            std::array<std::string, 2>{"TEXTURED", "NORMAL_MAPPED"});
    material._program2 = Program::from_files(
        "prepass.frag", "basic.vert", std::array<std::string, 2>{"TEXTURED", "NORMAL_MAPPED"});
    material._program3 =
        Program::from_files("prepass_debugocc.frag", "basic.vert",
                            std::array<std::string, 2>{"TEXTURED", "NORMAL_MAPPED"});
    return material;
}

} // namespace OM3D
