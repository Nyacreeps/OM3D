#version 450

#include "utils.glsl"
#line 4

#define ONE_SIXTH 0.16666666666666666
#define MN_B 0.333333333333
#define MN_C 0.333333333333
// 1 / 9 =
#define SAMPLE_COUNT_INV 0.1111111111111111
#define GAMMA 1.0

#define SOURCE_WEIGHT 0.05
#define HISTORY_WEIGHT 1.0 - SOURCE_WEIGHT

layout(location = 0) in vec2 in_uv;

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec4 out_color_history;

layout(binding = 0) uniform sampler2D in_color;
layout(binding = 1) uniform sampler2D in_velocity;
layout(binding = 2) uniform sampler2D in_depth;
layout(binding = 3) uniform sampler2D in_color_history;
layout(binding = 4) uniform sampler2D in_depth_history;

layout(binding = 0) uniform Data {
    FrameData frame;
};

layout(binding = 1) uniform TAA_Settings {
    TAASettings settings;
};

// adapted from https://en.wikipedia.org/wiki/Mitchell%E2%80%93Netravali_filters
float Mitchell_Netravali(float x) {
    if (x < 1){
        return ONE_SIXTH * (
            (12.0 - 9.0*MN_B - 6.0*MN_C) * pow(x, 3.0)
            + (-18.0 + 12.0*MN_B + 6*MN_C) * pow(x, 2.0)
            + (6.0 - 2.0*MN_B)
        );
    } else if (x < 2) {
        return ONE_SIXTH * (
            (-MN_B - 6.0*MN_C)*pow(x,3.0)
            + (6.0*MN_B + 30.0*MN_C * pow(x,2.0))
            + (-12.0*MN_B - 48.0*MN_C) * abs(x)
            + (8.0*MN_B + 24*MN_C)
        );
    }
    return 0.0;
}

// pulled from GDC presentation about Playdead's INSIDE's TAA technique
vec4 clip_aabb(
    vec3 aabb_min,  // cn_min
    vec3 aabb_max,  // cn_max
    vec4 p,         // c_in'
    vec4 q          // c_hist
)
{
    vec3 p_clip = 0.5 * (aabb_max + aabb_min);
    vec3 e_clip = 0.5 * (aabb_max - aabb_min);

    vec4 v_clip = q - vec4(p_clip, p.w);
    vec3 v_unit = v_clip.xyz / e_clip;
    vec3 a_unit = abs(v_unit);
    float ma_unit = max(a_unit.x, max(a_unit.y, a_unit.z));
    if (ma_unit > 1.0)
        return vec4(p_clip, p.w) + v_clip / ma_unit;
    else
        return q;// point inside aabb
}

float max_comp(vec3 v) {
    return max(max(v.r, v.g), v.b);
}

float max3(float a, float b, float c) {
    return max(max(a, b), c);
}

// A lot taken from: https://alextardif.com/TAA.html
// Seems like a decent resource
vec4 TAA_pixel_color() {
    vec3 neighbourhood_max = vec3(-99999);
    vec3 neighbourhood_min = vec3(99999);

    vec3 source_sample_total = vec3(0.0);
    float source_sample_total_weight = 0.0;

    vec3 acc = vec3(0);
    vec3 acc_sq = vec3(0);

    float closest_depth = 0.0;
    ivec2 closest_depth_pixel_position = ivec2(0);

    for (int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            ivec2 pixel_position = ivec2(gl_FragCoord.xy + vec2(x,y));
            pixel_position = clamp(pixel_position, ivec2(0), ivec2(settings.window_size) - 1);

            vec3 neighbour_color = max(vec3(0), texelFetch(in_color, pixel_position, 0).rgb);

            float sub_sample_distance = length(vec2(x,y));
            float sub_sample_weight = Mitchell_Netravali(sub_sample_distance);

            source_sample_total += neighbour_color * sub_sample_weight;
            source_sample_total_weight += sub_sample_weight;

            neighbourhood_max = max(neighbourhood_max, neighbour_color);
            neighbourhood_min = min(neighbourhood_min, neighbour_color);

            acc += neighbour_color;
            acc_sq += neighbour_color * neighbour_color;

            float current_depth = texelFetch(in_depth, pixel_position, 0).r;
            if (current_depth > closest_depth) {
                closest_depth = current_depth;
                closest_depth_pixel_position = pixel_position;
            }
        }
    }

    vec2 velocity = texelFetch(in_velocity, closest_depth_pixel_position, 0).xy;
    vec2 history_tex_coord = in_uv - velocity;
    vec3 source_sample = source_sample_total / source_sample_total_weight;

    if (any(notEqual(history_tex_coord, saturate(history_tex_coord)))) {
        return vec4(source_sample, 1.0);
    }

    vec3 history_sample = texture(in_color_history, history_tex_coord).rgb; // TODO replace with filtered sampling ?

    vec3 mu = acc * SAMPLE_COUNT_INV;
    vec3 sigma = sqrt(abs((acc_sq * SAMPLE_COUNT_INV) - (mu * mu)));
    vec3 minc = mu - GAMMA * sigma;
    vec3 maxc = mu + GAMMA * sigma;

    history_sample = clip_aabb(minc, maxc, vec4(clamp(source_sample, neighbourhood_min, neighbourhood_max), 1.0), vec4(history_sample, 1.0)).rgb;

    float source_weight = SOURCE_WEIGHT;
    float history_weight = HISTORY_WEIGHT;

    vec3 compressed_source = source_sample * (1.0/(max_comp(source_sample.rgb) + 1.0));
    vec3 compressed_history = history_sample * (1.0/(max_comp(history_sample.rgb)+1.0));

    float luminance_source = luminance(compressed_source);
    float luminance_history = luminance(compressed_history);

    source_weight *= 1.0 / (1.0 + luminance_source);
    history_weight *= 1.0 / (1.0 + luminance_history);

    vec3 result = (source_sample * source_weight + history_sample * history_weight) 
        / max3(source_weight, history_weight, 0.00001);

    return vec4(result, 1.0);
}

void main() {
    vec4 new_color = TAA_pixel_color();
    out_color = new_color;
    out_color_history = new_color;
}