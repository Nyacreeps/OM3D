#include "SceneObject.h"

#include <glm/gtc/matrix_transform.hpp>
#include <limits>

namespace OM3D {

SceneObject::SceneObject(std::shared_ptr<StaticMesh> mesh, std::shared_ptr<Material> material)
    : _mesh(std::move(mesh)), _material(std::move(material)) {
}



void SceneObject::render(const Frustum& frustum, const glm::vec3& camPosition) const {
    if (!_material || !_mesh) {
        return;
    }

    _material->set_uniform(HASH("model"), transform());
    _material->bind();
    _mesh->draw(frustum, _transform, camPosition);
}

float SceneObject::distToCam(const glm::vec3& camPos, const glm::vec3& camFront) const {
    float minDist = std::numeric_limits<float>::max();
    auto c = glm::vec3(this->transform() * glm::vec4(0.0, 0.0, 0.0, 1.0));
    for (int x = -1; x <= 1; x += 2) {
        for (int y = -1; y <= 1; y += 2) {
            for (int z = -1; z <= 1; z += 2) {
                auto t = glm::vec3(x * this->_mesh->lengthX / 2, y * this->_mesh->lengthY / 2,
                                   z * this->_mesh->lengthZ / 2);
                auto p = c + glm::mat3(transform()) * (t);
                float dist = std::abs(glm::dot((p - camPos), camFront));
                minDist = (dist < minDist ? dist : minDist);
            }
        }
    }
    return minDist;
}

void SceneObject::set_transform(const glm::mat4& tr) {
    _transform = tr;
}

const glm::mat4& SceneObject::transform() const {
    return _transform;
}

} // namespace OM3D
