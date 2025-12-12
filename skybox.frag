#version 460 core
#extension GL_ARB_bindless_texture : require

// Input from vertex shader
in vec2 tex_coord;

// Output
layout (location = 0) out vec4 FragColor;

// Uniforms
uniform mat4 inv_VP;  // Inverse of View-Projection matrix
uniform uvec2 skybox_texture;  // Bindless texture handle

void main()
{
    // Convert screen coordinates to world-space ray direction
    vec4 ndc = vec4(tex_coord * 2.0 - 1.0, 1.0, 1.0);
    vec4 world_pos = inv_VP * ndc;
    vec3 ray_dir = normalize(world_pos.xyz / world_pos.w);

    // Simple UV mapping for background image
    // Map the ray direction to UV coordinates
    // Horizontal: based on XY angle, Vertical: based on Z
    float u = 0.5 + atan(ray_dir.x, ray_dir.y) / 6.28318;  // 0 to 1
    float v = 0.5 - asin(clamp(ray_dir.z, -1.0, 1.0)) / 3.14159;  // 0 to 1

    vec2 uv = vec2(u, v);

    // Sample the texture
    vec4 sky_color;
    if (skybox_texture != uvec2(0)) {
        sky_color = texture(sampler2D(skybox_texture), uv);
    } else {
        // Fallback gradient: blue sky to warm horizon
        float t = ray_dir.z * 0.5 + 0.5;
        vec3 horizon = vec3(0.9, 0.7, 0.5);
        vec3 zenith = vec3(0.3, 0.5, 0.9);
        sky_color = vec4(mix(horizon, zenith, t), 1.0);
    }

    FragColor = sky_color;
}
