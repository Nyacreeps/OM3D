#ifndef STATICMESH_H
#define STATICMESH_H

#include <graphics.h>
#include <TypedBuffer.h>
#include <Vertex.h>

#include <vector>

#include "Camera.h"

namespace OM3D {

struct MeshData {
    std::vector<Vertex> vertices;
    std::vector<u32> indices;
};

class StaticMesh : NonCopyable {

    public:
        StaticMesh() = default;
        StaticMesh(StaticMesh&&) = default;
        StaticMesh& operator=(StaticMesh&&) = default;

        StaticMesh(const MeshData& data);

        void draw(const Frustum& frustum, const glm::mat4&, const glm::vec3 &posistion) const;

    private:
        TypedBuffer<Vertex> _vertex_buffer;
        TypedBuffer<u32> _index_buffer;
        float boundingSphereRadius;
};

}

#endif // STATICMESH_H
