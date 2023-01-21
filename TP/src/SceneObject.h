#ifndef SCENEOBJECT_H
#define SCENEOBJECT_H

#include <StaticMesh.h>
#include <Material.h>

#include <memory>

#include <glm/matrix.hpp>

#include "Camera.h"

namespace OM3D {

class SceneObject : NonCopyable {

    public:
        SceneObject(std::shared_ptr<StaticMesh> mesh = nullptr, std::shared_ptr<Material> material = nullptr);

        void render(const Frustum& frustum, const glm::vec3 &position) const;

        float distToCam(const glm::vec3& camPos, const glm::vec3& camFront) const;
        void set_transform(const glm::mat4& tr);
        const glm::mat4& transform() const;

        std::shared_ptr<StaticMesh> _mesh;
        std::shared_ptr<Material> _material;
        uint _queryId;
        bool _queryActive;
        bool mark = false;

    private:
        glm::mat4 _transform = glm::mat4(1.0f);
};

}

#endif // SCENEOBJECT_H
