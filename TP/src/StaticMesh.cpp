#include "StaticMesh.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <limits>
#include <iostream>

namespace OM3D {

StaticMesh::StaticMesh(const MeshData& data)
    : _vertex_buffer(data.vertices), _index_buffer(data.indices) {
    float maxDist = 0.0;
    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::min();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::min();
    float minZ = std::numeric_limits<float>::max();
    float maxZ = std::numeric_limits<float>::min();
    for (auto vertex : data.vertices) {
        minX = (vertex.position.x < minX ? vertex.position.x : minX);
        maxX = (vertex.position.x > maxX ? vertex.position.x : maxX);
        minY = (vertex.position.y < minY ? vertex.position.y : minY);
        maxY = (vertex.position.y > maxY ? vertex.position.y : maxY);
        minZ = (vertex.position.z < minZ ? vertex.position.z : minZ);
        maxZ = (vertex.position.z > maxZ ? vertex.position.z : maxZ);
        float dist = glm::l2Norm(vertex.position);
        if (dist > maxDist) maxDist = dist;
    }
    boundingSphereRadius = maxDist;
    lengthX = maxX - minX;
    lengthY = maxY - minY;
    lengthZ = maxZ - minZ;
    _data = data;
}

StaticMesh StaticMesh::CubeMesh() {
    std::vector<Vertex> vertices = {
        {{1, -1, -1}, {0, -1, -0}, {0.625, 0.5}},   {{1, -1, -1}, {0, 0, -1}, {0.625, 0.5}},
        {{1, -1, -1}, {1, 0, -0}, {0.625, 0.5}},    {{1, -1, 1}, {0, -1, -0}, {0.375, 0.5}},
        {{1, -1, 1}, {0, 0, 1}, {0.375, 0.5}},      {{1, -1, 1}, {1, 0, -0}, {0.375, 0.5}},
        {{1, 1, -1}, {0, 0, -1}, {0.625, 0.25}},    {{1, 1, -1}, {0, 1, -0}, {0.625, 0.25}},
        {{1, 1, -1}, {1, 0, -0}, {0.625, 0.25}},    {{1, 1, 1}, {0, 0, 1}, {0.375, 0.25}},
        {{1, 1, 1}, {0, 1, -0}, {0.375, 0.25}},     {{1, 1, 1}, {1, 0, -0}, {0.375, 0.25}},
        {{-1, -1, -1}, {-1, 0, -0}, {0.625, 0.75}}, {{-1, -1, -1}, {0, -1, -0}, {0.625, 0.75}},
        {{-1, -1, -1}, {0, 0, -1}, {0.875, 0.5}},   {{-1, -1, 1}, {-1, 0, -0}, {0.375, 0.75}},
        {{-1, -1, 1}, {0, -1, -0}, {0.375, 0.75}},  {{-1, -1, 1}, {0, 0, 1}, {0.125, 0.5}},
        {{-1, 1, -1}, {-1, 0, -0}, {0.625, 1}},     {{-1, 1, -1}, {0, 0, -1}, {0.875, 0.25}},
        {{-1, 1, -1}, {0, 1, -0}, {0.625, 0}},      {{-1, 1, 1}, {-1, 0, -0}, {0.375, 1}},
        {{-1, 1, 1}, {0, 0, 1}, {0.125, 0.25}},     {{-1, 1, 1}, {0, 1, -0}, {0.375, 0}}};

    std::vector<u32> indices = {14, 6,  1, 7, 23, 10, 18, 15, 21, 4, 22, 17, 2, 11, 5,  13, 3, 16,
                                14, 19, 6, 7, 20, 23, 18, 12, 15, 4, 9,  22, 2, 8,  11, 13, 0, 3};
    return StaticMesh({vertices, indices});
}

void StaticMesh::draw(const Frustum& frustum, const glm::mat4& transform,
                      const glm::vec3& camPosition) const {
    _vertex_buffer.bind(BufferUsage::Attribute);
    _index_buffer.bind(BufferUsage::Index);
    glm::vec3 center = glm::vec3(transform * glm::vec4(0.0, 0.0, 0.0, 1.0)) - camPosition;
    auto normals =
        std::vector<glm::vec3>{frustum._bottom_normal, frustum._left_normal, frustum._near_normal,
                               frustum._right_normal, frustum._top_normal};
    for (auto normal : normals) {
        if (glm::dot(normal, center + normal * boundingSphereRadius) < 0) return;
    }

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

    glDrawElements(GL_TRIANGLES, int(_index_buffer.element_count()), GL_UNSIGNED_INT, nullptr);
}

} // namespace OM3D
