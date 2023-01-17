#version 450

#include "utils.glsl"

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;
layout(location = 3) in vec4 in_tangent_bitangent_sign;
layout(location = 4) in vec3 in_color;
layout(location = 5) in mat4 model;
layout(location = 9) in vec3 light_pos;
layout(location = 10) in vec3 light_color;
layout(location = 11) in float light_radius;

layout(location = 0) out vec3 out_light_pos;
layout(location = 1) out vec3 out_light_color;
layout(location = 2) out float out_light_radius;

layout(binding = 0) uniform Data {
    FrameData frame;
};

void main() {
    const vec4 position = model * vec4(in_pos, 1.0);

    out_light_pos = light_pos;
    out_light_color = light_color;
    out_light_radius = light_radius;

    gl_Position = frame.camera.view_proj * position;
}

