#include "Scene.h"

#include <TypedBuffer.h>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <iostream>
#include <map>
#include <shader_structs.h>

namespace OM3D {

Scene::Scene() {}

void Scene::add_object(SceneObject obj) {
  _objects.emplace_back(std::move(obj));
}

void Scene::add_object(PointLight obj) {
  _point_lights.emplace_back(std::move(obj));
}

void Scene::render(const Camera &camera) const {
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
  TypedBuffer<shader::PointLight> light_buffer(
      nullptr, std::max(_point_lights.size(), size_t(1)));
  {
    auto mapping = light_buffer.map(AccessType::WriteOnly);
    for (size_t i = 0; i != _point_lights.size(); ++i) {
      const auto &light = _point_lights[i];
      mapping[i] = {light.position(), light.radius(), light.color(), 0.0f};
    }
  }
  light_buffer.bind(BufferUsage::Storage, 1);

  // Render every object
  // NON INSTANCED DRAW
  /* for(const SceneObject& obj : _objects) {
      obj.render(camera.build_frustum(), camera.position());
  } */

  // INSTANCED DRAW (stupid)
  struct InstanceDrawData {
    std::vector<Instance> instanceVertices;
    std::shared_ptr<StaticMesh> mesh;
    std::shared_ptr<Material> mat;
  };

  std::map<int, InstanceDrawData> instanceGroups;
  // getting each unique material
  for (const SceneObject &obj : _objects) {
    int isCulled = false;
    glm::vec3 center = glm::vec3(obj.transform() * glm::vec4(0.0, 0.0, 0.0, 1.0)) - camera.position();
    auto frustum = camera.build_frustum();
    auto normals = std::vector<glm::vec3>{frustum._bottom_normal, frustum._left_normal, frustum._near_normal, frustum._right_normal, frustum._top_normal};
    for (auto normal : normals) {
        if (glm::dot(normal, center + normal * obj._mesh->boundingSphereRadius) < 0)
            isCulled = true;
    }
    if (isCulled)
        continue;
    instanceGroups[obj._material->uid].instanceVertices.push_back(
        {obj.transform()});
    instanceGroups[obj._material->uid].mat = obj._material;
    instanceGroups[obj._material->uid].mesh = obj._mesh;
    // we should have the mesh as a key, but it seems like 2 identical cubes
    // will have 2 different StaticMesh instances here
  }

  for (auto &[key, value] : instanceGroups) {
    if (!value.mat || !value.mesh)
      continue;
    value.mat->bind();
    TypedBuffer<Instance> instanceBuffer(value.instanceVertices);

    value.mesh->_vertex_buffer.bind(BufferUsage::Attribute);
    value.mesh->_index_buffer.bind(BufferUsage::Index);

    // Vertex position
    glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), nullptr);
    // Vertex normal
    glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(Vertex),
                          reinterpret_cast<void *>(3 * sizeof(float)));
    // Vertex uv
    glVertexAttribPointer(2, 2, GL_FLOAT, false, sizeof(Vertex),
                          reinterpret_cast<void *>(6 * sizeof(float)));
    // Tangent / bitangent sign
    glVertexAttribPointer(3, 4, GL_FLOAT, false, sizeof(Vertex),
                          reinterpret_cast<void *>(8 * sizeof(float)));
    // Vertex color
    glVertexAttribPointer(4, 3, GL_FLOAT, false, sizeof(Vertex),
                          reinterpret_cast<void *>(12 * sizeof(float)));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);

    instanceBuffer.bind(BufferUsage::Attribute);

    glVertexAttribPointer(5, 4, GL_FLOAT, false, sizeof(Instance), 0);
    glVertexAttribPointer(6, 4, GL_FLOAT, false, sizeof(Instance),
                          (void *)(4 * sizeof(GLfloat)));
    glVertexAttribPointer(7, 4, GL_FLOAT, false, sizeof(Instance),
                          (void *)(8 * sizeof(GLfloat)));
    glVertexAttribPointer(8, 4, GL_FLOAT, false, sizeof(Instance),
                          (void *)(12 * sizeof(GLfloat)));

    glEnableVertexAttribArray(5);
    glEnableVertexAttribArray(6);
    glEnableVertexAttribArray(7);
    glEnableVertexAttribArray(8);

    glVertexAttribDivisor(5, 1);
    glVertexAttribDivisor(6, 1);
    glVertexAttribDivisor(7, 1);
    glVertexAttribDivisor(8, 1);

    glDrawElementsInstanced(GL_TRIANGLES,
                            int(value.mesh->_index_buffer.element_count()),
                            GL_UNSIGNED_INT, 0, value.instanceVertices.size());
  }
}

} // namespace OM3D
