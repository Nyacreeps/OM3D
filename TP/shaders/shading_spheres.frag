#version 450

#include "utils.glsl"

layout(location = 0) in vec3 light_pos;
layout(location = 1) in vec3 light_color;
layout(location = 2) in float light_radius;

layout(location = 0) out vec4 out_color;

layout(binding = 0) uniform sampler2D in_albedo;
layout(binding = 1) uniform sampler2D in_normal;
layout(binding = 2) uniform sampler2D in_depth;

layout(binding = 0) uniform Data {
    FrameData frame;
};

layout(binding = 1) buffer PointLights {
    PointLight point_lights[];
};

const vec3 ambient = vec3(0.0);

vec3 unproject(vec2 uv, float depth, mat4 inv_viewproj) {
    const vec3 ndc = vec3(uv * 2.0 - vec2(1.0), depth);
    const vec4 p = inv_viewproj * vec4(ndc, 1.0);
    return p.xyz / p.w;
}

void main() {
    vec3 albedo = texelFetch(in_albedo, ivec2(gl_FragCoord.xy), 0).rgb;
    vec3 normal = texelFetch(in_normal, ivec2(gl_FragCoord.xy), 0).rgb;
    normal = normal * 2.0 - vec3(1.0);
    float depth = texelFetch(in_normal, ivec2(gl_FragCoord.xy), 0).r;
    vec3 position = unproject(gl_FragCoord.xy, depth, inverse(frame.camera.view_proj));

    const vec3 to_light = (light_pos - position);
    const float dist = length(to_light);
    const vec3 light_vec = to_light / dist;

    const float NoL = max(dot(light_vec, normal), 0.0);
    const float att = attenuation(dist, light_radius);

    vec3 acc = light_color;

    out_color = vec4(albedo * acc, 1.0);
}