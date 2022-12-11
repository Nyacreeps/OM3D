#version 450

layout(location = 0) in vec2 in_uv;

layout(location = 0) out vec4 out_color;

layout(binding = 0) uniform sampler2D in_texture;

layout(location = 0) uniform int in_debug_state;

void main() {
    out_color = texture(in_texture, in_uv);
}