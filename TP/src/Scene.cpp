#include "Scene.h"

#include <TypedBuffer.h>
#include "graphics.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <iostream>
#include <map>
#include <shader_structs.h>
#include <algorithm>

namespace OM3D {

Scene::Scene() {
}

void Scene::add_object(SceneObject obj) {
    _objects.emplace_back(std::move(obj));
}

void Scene::add_object(PointLight obj) {
    _point_lights.emplace_back(std::move(obj));
}

void Scene::sortObjects(const Camera& camera) {
    std::sort(_objects.begin(), _objects.end(),
              [&](const SceneObject& lhs, const SceneObject& rhs) {
                  return lhs.distToCam(camera.position(), glm::normalize(camera.forward())) <
                         rhs.distToCam(camera.position(), glm::normalize(camera.forward()));
              });
}

/* void Scene::moveObjects(double time) {
    for (auto& obj : _objects) {
        if (obj._move) {
            obj.set_transform(glm::translate(obj.transform(), obj._move(time)));
        }
    }
} */

void Scene::renderShading(const Camera& camera, std::shared_ptr<Program> programp) const {
    // Fill and bind frame data buffer
    TypedBuffer<shader::FrameData> buffer(nullptr, 1);
    {
        auto mapping = buffer.map(AccessType::WriteOnly);
        mapping[0].camera.view_proj = camera.view_proj_matrix();
        mapping[0].point_light_count = u32(_point_lights.size());
        mapping[0].sun_color = glm::vec3(1.0f, 1.0f, 1.0f);
        mapping[0].sun_dir = glm::normalize(_sun_direction);
    }
    buffer.bind(BufferUsage::Uniform, 0);

    // Fill and bind lights buffer
    TypedBuffer<shader::PointLight> light_buffer(nullptr,
                                                 std::max(_point_lights.size(), size_t(1)));
    {
        auto mapping = light_buffer.map(AccessType::WriteOnly);
        for (size_t i = 0; i != _point_lights.size(); ++i) {
            const auto& light = _point_lights[i];
            mapping[i] = {light.position(), light.radius(), light.color(), 0.0f};
        }
    }
    light_buffer.bind(BufferUsage::Storage, 1);

    auto mat = Material();
    mat.set_blend_mode(BlendMode::None);
    mat.set_depth_test_mode(DepthTestMode::None);
    mat.set_depth_mask_mode(DepthMaskMode::False);
    mat.set_program(programp);
    mat.bind(RenderMode::INSTANCED);

    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void Scene::renderShadingSpheres(const Camera& camera, std::shared_ptr<Program> programp) const {
    // Fill and bind frame data buffer
    TypedBuffer<shader::FrameData> buffer(nullptr, 1);
    {
        auto mapping = buffer.map(AccessType::WriteOnly);
        mapping[0].camera.view_proj = camera.view_proj_matrix();
        mapping[0].point_light_count = u32(_point_lights.size());
        mapping[0].sun_color = glm::vec3(1.0f, 1.0f, 1.0f);
        mapping[0].sun_dir = glm::normalize(_sun_direction);
    }
    buffer.bind(BufferUsage::Uniform, 0);

    // Fill and bind lights buffer
    TypedBuffer<shader::PointLight> light_buffer(nullptr,
                                                 std::max(_point_lights.size(), size_t(1)));
    {
        auto mapping = light_buffer.map(AccessType::WriteOnly);
        for (size_t i = 0; i != _point_lights.size(); ++i) {
            const auto& light = _point_lights[i];
            mapping[i] = {light.position(), light.radius(), light.color(), 0.0f};
        }
    }
    light_buffer.bind(BufferUsage::Storage, 1);

    static auto sphereMeshp = meshFromGltf(std::string(data_path) + "sphere.glb").value;
    sphereMeshp->_vertex_buffer.bind(BufferUsage::Attribute);
    sphereMeshp->_index_buffer.bind(BufferUsage::Index);

    auto mat = Material();
    mat.set_blend_mode(BlendMode::Additive);
    mat.set_depth_test_mode(DepthTestMode::Reversed);
    mat.set_depth_mask_mode(DepthMaskMode::False);
    mat.set_program(programp);
    mat.bind(RenderMode::INSTANCED);

    // Vertex position
    glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), nullptr);
    // Vertex normal
    glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(Vertex),
                          reinterpret_cast<void*>(3 * sizeof(float)));
    // Vertex uv
    glVertexAttribPointer(2, 2, GL_FLOAT, false, sizeof(Vertex),
                          reinterpret_cast<void*>(6 * sizeof(float)));
    // Tangent / bitangent sign
    glVertexAttribPointer(3, 4, GL_FLOAT, false, sizeof(Vertex),
                          reinterpret_cast<void*>(8 * sizeof(float)));
    // Vertex color
    glVertexAttribPointer(4, 3, GL_FLOAT, false, sizeof(Vertex),
                          reinterpret_cast<void*>(12 * sizeof(float)));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);

    std::vector<LightInstance> instanceVertices;
    for (auto& pointLight : this->_point_lights) {
        glm::mat4 trans = glm::translate(glm::mat4(1.0), pointLight.position());
        glm::mat4 scale = glm::scale(glm::mat4(1.0), glm::vec3(pointLight.radius() * 0.1f));
        instanceVertices.push_back(
            {trans * scale, pointLight.position(), pointLight.color(), pointLight.radius()});
    }
    TypedBuffer<LightInstance> instanceBuffer(instanceVertices);
    instanceBuffer.bind(BufferUsage::Attribute);
    glVertexAttribPointer(5, 4, GL_FLOAT, false, sizeof(LightInstance), 0);
    glVertexAttribPointer(6, 4, GL_FLOAT, false, sizeof(LightInstance),
                          (void*)(4 * sizeof(GLfloat)));
    glVertexAttribPointer(7, 4, GL_FLOAT, false, sizeof(LightInstance),
                          (void*)(8 * sizeof(GLfloat)));
    glVertexAttribPointer(8, 4, GL_FLOAT, false, sizeof(LightInstance),
                          (void*)(12 * sizeof(GLfloat)));
    glVertexAttribPointer(9, 3, GL_FLOAT, false, sizeof(LightInstance),
                          (void*)offsetof(LightInstance, pos));
    glVertexAttribPointer(10, 3, GL_FLOAT, false, sizeof(LightInstance),
                          (void*)offsetof(LightInstance, color));
    glVertexAttribPointer(11, 1, GL_FLOAT, false, sizeof(LightInstance),
                          (void*)offsetof(LightInstance, radius));
    glEnableVertexAttribArray(5);
    glEnableVertexAttribArray(6);
    glEnableVertexAttribArray(7);
    glEnableVertexAttribArray(8);
    glEnableVertexAttribArray(9);
    glEnableVertexAttribArray(10);
    glEnableVertexAttribArray(11);

    glVertexAttribDivisor(5, 1);
    glVertexAttribDivisor(6, 1);
    glVertexAttribDivisor(7, 1);
    glVertexAttribDivisor(8, 1);
    glVertexAttribDivisor(9, 1);
    glVertexAttribDivisor(10, 1);
    glVertexAttribDivisor(11, 1);

    glDrawElementsInstanced(GL_TRIANGLES, int(sphereMeshp->_index_buffer.element_count()),
                            GL_UNSIGNED_INT, 0, instanceVertices.size());
}

void Scene::renderShadingDirectional(const Camera& camera,
                                     std::shared_ptr<Program> programp) const {
    // Fill and bind frame data buffer
    TypedBuffer<shader::FrameData> buffer(nullptr, 1);
    {
        auto mapping = buffer.map(AccessType::WriteOnly);
        mapping[0].camera.view_proj = camera.view_proj_matrix();
        mapping[0].point_light_count = u32(_point_lights.size());
        mapping[0].sun_color = glm::vec3(1.0f, 1.0f, 1.0f);
        mapping[0].sun_dir = glm::normalize(_sun_direction);
    }
    buffer.bind(BufferUsage::Uniform, 0);

    // Fill and bind lights buffer
    TypedBuffer<shader::PointLight> light_buffer(nullptr,
                                                 std::max(_point_lights.size(), size_t(1)));
    {
        auto mapping = light_buffer.map(AccessType::WriteOnly);
        for (size_t i = 0; i != _point_lights.size(); ++i) {
            const auto& light = _point_lights[i];
            mapping[i] = {light.position(), light.radius(), light.color(), 0.0f};
        }
    }
    light_buffer.bind(BufferUsage::Storage, 1);

    auto mat = Material();
    mat.set_blend_mode(BlendMode::None);
    mat.set_depth_test_mode(DepthTestMode::None);
    mat.set_depth_mask_mode(DepthMaskMode::False);
    mat.set_program(programp);
    mat.bind(RenderMode::INSTANCED);

    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void Scene::render(const Camera& camera) const {
    // Fill and bind frame data buffer
    TypedBuffer<shader::FrameData> buffer(nullptr, 1);
    {
        auto mapping = buffer.map(AccessType::WriteOnly);
        mapping[0].camera.view_proj = camera.view_proj_matrix();
        mapping[0].point_light_count = u32(_point_lights.size());
        mapping[0].sun_color = glm::vec3(1.0f, 1.0f, 1.0f);
        mapping[0].sun_dir = glm::normalize(_sun_direction);
    }
    buffer.bind(BufferUsage::Uniform, 0);

    // Fill and bind lights buffer
    TypedBuffer<shader::PointLight> light_buffer(nullptr,
                                                 std::max(_point_lights.size(), size_t(1)));
    {
        auto mapping = light_buffer.map(AccessType::WriteOnly);
        for (size_t i = 0; i != _point_lights.size(); ++i) {
            const auto& light = _point_lights[i];
            mapping[i] = {light.position(), light.radius(), light.color(), 0.0f};
        }
    }
    light_buffer.bind(BufferUsage::Storage, 1);

    // INSTANCED DRAW (stupid)
    struct InstanceDrawData {
        std::vector<Instance> instanceVertices;
        std::shared_ptr<StaticMesh> mesh;
        std::shared_ptr<Material> mat;
    };

    std::map<int, InstanceDrawData> instanceGroups;
    // getting each unique material
    for (const SceneObject& obj : _objects) {
        int isCulled = false;
        auto transform = obj.transform();
        glm::vec3 center = glm::vec3(transform * glm::vec4(0.0, 0.0, 0.0, 1.0)) - camera.position();
        float scaling =
            std::sqrt(std::pow(transform[0][0], 2.0f) + std::pow(transform[0][1], 2.0f) +
                      std::pow(transform[0][2], 2.0f));
        auto frustum = camera.build_frustum();
        auto normals = std::vector<glm::vec3>{frustum._bottom_normal, frustum._left_normal,
                                              frustum._near_normal, frustum._right_normal,
                                              frustum._top_normal};
        for (auto normal : normals) {
            if (glm::dot(normal, center + normal * obj._mesh->boundingSphereRadius * scaling) < 0)
                isCulled = true;
        }
        if (isCulled) continue;
        instanceGroups[obj._material->uid].instanceVertices.push_back({obj.transform()});
        instanceGroups[obj._material->uid].mat = obj._material;
        instanceGroups[obj._material->uid].mesh = obj._mesh;
    }

    for (auto& [key, value] : instanceGroups) {
        if (!value.mat || !value.mesh) continue;
        value.mat->bind(RenderMode::INSTANCED);
        TypedBuffer<Instance> instanceBuffer(value.instanceVertices);

        value.mesh->_vertex_buffer.bind(BufferUsage::Attribute);
        value.mesh->_index_buffer.bind(BufferUsage::Index);

        // Vertex position
        glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), nullptr);
        // Vertex normal
        glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(Vertex),
                              reinterpret_cast<void*>(3 * sizeof(float)));
        // Vertex uv
        glVertexAttribPointer(2, 2, GL_FLOAT, false, sizeof(Vertex),
                              reinterpret_cast<void*>(6 * sizeof(float)));
        // Tangent / bitangent sign
        glVertexAttribPointer(3, 4, GL_FLOAT, false, sizeof(Vertex),
                              reinterpret_cast<void*>(8 * sizeof(float)));
        // Vertex color
        glVertexAttribPointer(4, 3, GL_FLOAT, false, sizeof(Vertex),
                              reinterpret_cast<void*>(12 * sizeof(float)));

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glEnableVertexAttribArray(3);
        glEnableVertexAttribArray(4);

        instanceBuffer.bind(BufferUsage::Attribute);

        glVertexAttribPointer(5, 4, GL_FLOAT, false, sizeof(Instance), 0);
        glVertexAttribPointer(6, 4, GL_FLOAT, false, sizeof(Instance),
                              (void*)(4 * sizeof(GLfloat)));
        glVertexAttribPointer(7, 4, GL_FLOAT, false, sizeof(Instance),
                              (void*)(8 * sizeof(GLfloat)));
        glVertexAttribPointer(8, 4, GL_FLOAT, false, sizeof(Instance),
                              (void*)(12 * sizeof(GLfloat)));

        glEnableVertexAttribArray(5);
        glEnableVertexAttribArray(6);
        glEnableVertexAttribArray(7);
        glEnableVertexAttribArray(8);

        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);
        glVertexAttribDivisor(7, 1);
        glVertexAttribDivisor(8, 1);

        glDrawElementsInstanced(GL_TRIANGLES, int(value.mesh->_index_buffer.element_count()),
                                GL_UNSIGNED_INT, 0, value.instanceVertices.size());
    }
}

void Scene::renderOcclusion(const Camera& camera, bool debug) {
    // Fill and bind frame data buffer
    TypedBuffer<shader::FrameData> buffer(nullptr, 1);
    {
        auto mapping = buffer.map(AccessType::WriteOnly);
        mapping[0].camera.view_proj = camera.view_proj_matrix();
        mapping[0].point_light_count = u32(_point_lights.size());
        mapping[0].sun_color = glm::vec3(1.0f, 1.0f, 1.0f);
        mapping[0].sun_dir = glm::normalize(_sun_direction);
    }
    buffer.bind(BufferUsage::Uniform, 0);

    // Fill and bind lights buffer
    TypedBuffer<shader::PointLight> light_buffer(nullptr,
                                                 std::max(_point_lights.size(), size_t(1)));
    {
        auto mapping = light_buffer.map(AccessType::WriteOnly);
        for (size_t i = 0; i != _point_lights.size(); ++i) {
            const auto& light = _point_lights[i];
            mapping[i] = {light.position(), light.radius(), light.color(), 0.0f};
        }
    }
    light_buffer.bind(BufferUsage::Storage, 1);

    // Render every object
    int a = 0;
    for (SceneObject& obj : _objects) {
        a++;
        if (!obj._material || !obj._mesh) {
            continue;
        }

        // frustum culling
        int isCulled = false;
        auto transform = obj.transform();
        glm::vec3 center = glm::vec3(transform * glm::vec4(0.0, 0.0, 0.0, 1.0)) - camera.position();
        float scaling =
            std::sqrt(std::pow(transform[0][0], 2.0f) + std::pow(transform[0][1], 2.0f) +
                      std::pow(transform[0][2], 2.0f));
        auto frustum = camera.build_frustum();
        auto normals = std::vector<glm::vec3>{frustum._bottom_normal, frustum._left_normal,
                                              frustum._near_normal, frustum._right_normal,
                                              frustum._top_normal};
        for (auto normal : normals) {
            if (glm::dot(normal, center + normal * obj._mesh->boundingSphereRadius * scaling) < 0)
                isCulled = true;
        }
        if (isCulled) continue;

        // occlusion culling

        int samplesPassed = 0;
        int resultAvailable = 0;
        if (obj._queryActive) {
            glGetQueryObjectiv(obj._queryId, GL_QUERY_RESULT_AVAILABLE, &resultAvailable);
            if (resultAvailable)
                glGetQueryObjectiv(obj._queryId, GL_QUERY_RESULT, &samplesPassed);
        }
        glBeginQuery(GL_SAMPLES_PASSED, obj._queryId);
        obj._queryActive = true;
        if (samplesPassed != 0) {
            if (obj.mark) std::cout << "center cube visible\n";
            obj._material->bind(RenderMode::NON_INSTANCED);
            obj._material->set_uniform(RenderMode::NON_INSTANCED, HASH("model"), obj.transform());
            obj._mesh->_vertex_buffer.bind(BufferUsage::Attribute);
            obj._mesh->_index_buffer.bind(BufferUsage::Index);

            // Vertex position
            glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), nullptr);
            // Vertex normal
            glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(Vertex),
                                  reinterpret_cast<void*>(3 * sizeof(float)));
            // Vertex uv
            glVertexAttribPointer(2, 2, GL_FLOAT, false, sizeof(Vertex),
                                  reinterpret_cast<void*>(6 * sizeof(float)));
            // Tangent / bitangent sign
            glVertexAttribPointer(3, 4, GL_FLOAT, false, sizeof(Vertex),
                                  reinterpret_cast<void*>(8 * sizeof(float)));
            // Vertex color
            glVertexAttribPointer(4, 3, GL_FLOAT, false, sizeof(Vertex),
                                  reinterpret_cast<void*>(12 * sizeof(float)));

            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);
            glEnableVertexAttribArray(2);
            glEnableVertexAttribArray(3);
            glEnableVertexAttribArray(4);
            glDrawElements(GL_TRIANGLES, int(obj._mesh->_index_buffer.element_count()),
                           GL_UNSIGNED_INT, nullptr);
        } else if (samplesPassed == 0) {
            if (obj.mark) std::cout << "center cube occluded\n";
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            glDepthMask(GL_FALSE);

            obj._material->bind(RenderMode::NON_INSTANCED);
            obj._material->set_uniform(RenderMode::NON_INSTANCED, HASH("model"), obj.transform());
            obj._mesh->_vertex_buffer.bind(BufferUsage::Attribute);
            obj._mesh->_index_buffer.bind(BufferUsage::Index);

            // Vertex position
            glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), nullptr);
            // Vertex normal
            glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(Vertex),
                                  reinterpret_cast<void*>(3 * sizeof(float)));
            // Vertex uv
            glVertexAttribPointer(2, 2, GL_FLOAT, false, sizeof(Vertex),
                                  reinterpret_cast<void*>(6 * sizeof(float)));
            // Tangent / bitangent sign
            glVertexAttribPointer(3, 4, GL_FLOAT, false, sizeof(Vertex),
                                  reinterpret_cast<void*>(8 * sizeof(float)));
            // Vertex color
            glVertexAttribPointer(4, 3, GL_FLOAT, false, sizeof(Vertex),
                                  reinterpret_cast<void*>(12 * sizeof(float)));

            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);
            glEnableVertexAttribArray(2);
            glEnableVertexAttribArray(3);
            glEnableVertexAttribArray(4);
            glDrawElements(GL_TRIANGLES, int(obj._mesh->_index_buffer.element_count()),
                           GL_UNSIGNED_INT, nullptr);

            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glDepthMask(GL_TRUE);
        }

        glEndQuery(GL_SAMPLES_PASSED);

        if (debug && samplesPassed == 0) {
            obj._material->bind(RenderMode::OCC_DEBUG);
            obj._material->set_uniform(RenderMode::OCC_DEBUG, HASH("model"), obj.transform());
            obj._mesh->_vertex_buffer.bind(BufferUsage::Attribute);
            obj._mesh->_index_buffer.bind(BufferUsage::Index);

            // Vertex position
            glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), nullptr);
            // Vertex normal
            glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(Vertex),
                                  reinterpret_cast<void*>(3 * sizeof(float)));
            // Vertex uv
            glVertexAttribPointer(2, 2, GL_FLOAT, false, sizeof(Vertex),
                                  reinterpret_cast<void*>(6 * sizeof(float)));
            // Tangent / bitangent sign
            glVertexAttribPointer(3, 4, GL_FLOAT, false, sizeof(Vertex),
                                  reinterpret_cast<void*>(8 * sizeof(float)));
            // Vertex color
            glVertexAttribPointer(4, 3, GL_FLOAT, false, sizeof(Vertex),
                                  reinterpret_cast<void*>(12 * sizeof(float)));

            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);
            glEnableVertexAttribArray(2);
            glEnableVertexAttribArray(3);
            glEnableVertexAttribArray(4);
            glDrawElements(GL_TRIANGLES, int(obj._mesh->_index_buffer.element_count()),
                           GL_UNSIGNED_INT, nullptr);
        }
    }
}

} // namespace OM3D
