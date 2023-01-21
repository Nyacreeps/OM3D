#version 450

#include "utils.glsl"

// #define DEBUG_NORMAL

layout(location = 0) out vec4 gAlbedo;
layout(location = 1) out vec4 gNormals;
layout(location = 2) out vec2 gVelocity;

layout(location = 0) in vec3 in_normal;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_color;
layout(location = 3) in vec3 in_position;
layout(location = 4) in vec3 in_tangent;
layout(location = 5) in vec3 in_bitangent;
layout(location = 6) in vec4 in_prev_camera_position;

layout(binding = 0) uniform sampler2D in_texture;
layout(binding = 1) uniform sampler2D in_normal_texture;

layout(binding = 0) uniform Data {
    FrameData frame;
};

layout(binding = 1) buffer PointLights {
    PointLight point_lights[];
};

const vec3 ambient = vec3(0.0);

void main() {
#ifdef NORMAL_MAPPED
    const vec3 normal_map = unpack_normal_map(texture(in_normal_texture, in_uv).xy);
    const vec3 normal = normal_map.x * in_tangent +
                        normal_map.y * in_bitangent +
                        normal_map.z * in_normal;
#else
    const vec3 normal = in_normal;
#endif
    gAlbedo = vec4(in_color, 1.0);
    gNormals = vec4((in_normal + vec3(1.0)) / 2.0, 1.0);
    vec2 current_pos = (gl_FragCoord.xy/gl_FragCoord.w);
    vec2 previous_pos = (in_prev_camera_position.xy/in_prev_camera_position.w);
    gVelocity = (current_pos - frame.camera.jitter) - (previous_pos - frame.camera.prev_jitter);

#ifdef TEXTURED
    gAlbedo *= texture(in_texture, in_uv);
#endif
}