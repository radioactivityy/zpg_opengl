#version 460 core
#extension GL_ARB_bindless_texture : require

in vec2 tex_coord;
layout (location = 0) out vec4 FragColor;

uniform mat4 inv_VP;
uniform uvec2 skybox_texture;

const float PI = 3.14159265359;

void main()
{
    // Convert screen coordinates to world-space ray direction
    vec4 ndc = vec4(tex_coord * 2.0 - 1.0, 1.0, 1.0);
    vec4 world_pos = inv_VP * ndc;
    vec3 ray_dir = normalize(world_pos.xyz / world_pos.w);

    // Cylindrical projection for 180-degree horizontal FOV
    // Only use horizontal angle (yaw), keep vertical natural
    float yaw = atan(ray_dir.y, ray_dir.x);  // -PI to PI

    // Map yaw to U coordinate (180 degrees = full image width)
    float u = (yaw / PI) * 0.5 + 0.5;  // 0 to 1 for 180 degrees

    // For V, use gentle pitch mapping for natural horizon look
    float pitch = asin(clamp(ray_dir.z, -1.0, 1.0));
    float v = (pitch / (PI * 0.5)) * 0.5 + 0.5;  // 0 to 1

    // Clamp to valid range
    u = clamp(u, 0.0, 1.0);
    v = clamp(v, 0.0, 1.0);

    vec2 uv = vec2(u, v);

    // Sample skybox texture
    vec4 sky_color = vec4(0.4, 0.6, 0.9, 1.0);  // Default sky blue
    if (skybox_texture != uvec2(0)) {
        sky_color = texture(sampler2D(skybox_texture), uv);
    }

    FragColor = sky_color;
}
