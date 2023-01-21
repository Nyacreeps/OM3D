struct CameraData {
    mat4 view_proj;
    mat4 prev_view_proj;
    vec2 jitter;
    vec2 prev_jitter;
};

struct FrameData {
    CameraData camera;

    vec3 sun_dir;
    uint point_light_count;

    vec3 sun_color;
    float padding_1;
};

struct PointLight {
    vec3 position;
    float radius;
    vec3 color;
    float padding_1;
};

struct TAASettings {
    uvec2 window_size;
    uvec2 padding_1;
};
