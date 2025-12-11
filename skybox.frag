#version 460 core
#extension GL_ARB_bindless_texture : require

// Input from vertex shader
in vec2 tex_coord;

// Output
layout (location = 0) out vec4 FragColor;

// Uniforms
uniform mat4 inv_VP;  // Inverse of View-Projection matrix
uniform uvec2 skybox_texture;  // Bindless texture handle

// Constants
const float PI = 3.14159265359;

void main()
{
    // Convert screen coordinates to world-space ray direction
    // NDC coordinates
    vec4 ndc = vec4(tex_coord * 2.0 - 1.0, 1.0, 1.0);

    // Transform to world space
    vec4 world_pos = inv_VP * ndc;
    vec3 ray_dir = normalize(world_pos.xyz / world_pos.w);

    // Convert ray direction to equirectangular UV coordinates
    // Spherical coordinates: theta (azimuth), phi (elevation)
    float theta = atan(ray_dir.y, ray_dir.x);  // -PI to PI
    float phi = asin(clamp(ray_dir.z, -1.0, 1.0));  // -PI/2 to PI/2

    // Convert to UV (0-1 range)
    vec2 uv;
    uv.x = (theta + PI) / (2.0 * PI);  // 0 to 1
    uv.y = 1.0 - ((phi + PI / 2.0) / PI);       // 0 to 1

    // Sample the skybox texture
    vec4 sky_color = vec4(0.3, 0.5, 0.8, 1.0);  // Default sky blue
    if (skybox_texture != uvec2(0)) {
        sky_color = texture(sampler2D(skybox_texture), uv);
    }

    // Output the sky color
    FragColor = sky_color;
}
