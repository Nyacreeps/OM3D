#version 450

#include "utils.glsl"

layout(location = 0) out vec4 out_color;

layout(location = 0) in vec2 in_uv;

layout(binding = 1) uniform sampler2D in_albedo;
layout(binding = 2) uniform sampler2D in_normal;
layout(binding = 3) uniform sampler2D in_depth;

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

    vec3 acc = frame.sun_color * max(0.0, dot(frame.sun_dir, normal)) + ambient;

    for(uint i = 0; i != frame.point_light_count; ++i) {
        PointLight light = point_lights[i];
        const vec3 to_light = (light.position - position);
        const float dist = length(to_light);
        const vec3 light_vec = to_light / dist;

        const float NoL = dot(light_vec, normal);
        const float att = attenuation(dist, light.radius);
        if(NoL <= 0.0 || att <= 0.0f) {
            continue;
        }

        acc += light.color * (NoL * att);
    }

    out_color = vec4(albedo * acc, 0.0);
}