#include "StaticMesh.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

namespace OM3D {



StaticMesh::StaticMesh(const MeshData& data) :
    _vertex_buffer(data.vertices),
    _index_buffer(data.indices) {
        float maxDist = 0.0;
        for (auto vertex : data.vertices) {
            float dist = glm::l2Norm(vertex.position);
            if (dist > maxDist) maxDist = dist;
        }
        boundingSphereRadius = maxDist;
}

void StaticMesh::draw(const Frustum& frustum, const glm::mat4& transform, const glm::vec3 &camPosition) const {
    _vertex_buffer.bind(BufferUsage::Attribute);
    _index_buffer.bind(BufferUsage::Index);
    glm::vec3 center = glm::vec3(transform * glm::vec4(0.0, 0.0, 0.0, 1.0)) - camPosition;
    auto normals = std::vector<glm::vec3>{frustum._bottom_normal, frustum._left_normal, frustum._near_normal, frustum._right_normal, frustum._top_normal};
    for (auto normal : normals) {
        if (glm::dot(normal, center + normal * boundingSphereRadius) < 0)
            return;
    }

    // Vertex position
    glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), nullptr);
    // Vertex normal
    glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(3 * sizeof(float)));
    // Vertex uv
    glVertexAttribPointer(2, 2, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(6 * sizeof(float)));
    // Tangent / bitangent sign
    glVertexAttribPointer(3, 4, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(8 * sizeof(float)));
    // Vertex color
    glVertexAttribPointer(4, 3, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void*>(12 * sizeof(float)));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);

    glDrawElements(GL_TRIANGLES, int(_index_buffer.element_count()), GL_UNSIGNED_INT, nullptr);
}

}
