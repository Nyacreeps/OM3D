#ifndef VERTEX_H
#define VERTEX_H

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

namespace OM3D {

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
    glm::vec4 tangent_bitangent_sign = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f); // to avoid completly black meshes if no color is present
};

struct Instance {
    glm::mat4 model = glm::mat4(1.0);
};

struct LightInstance {
    glm::mat4 model = glm::mat4(1.0);
    glm::vec3 pos;
    glm::vec3 color;
    float radius;
};

}

#endif // VERTEX_H
