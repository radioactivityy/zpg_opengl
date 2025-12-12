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
    vec4 ndc = vec4(tex_coord * 2.0 - 1.0, 1.0, 1.0);
    vec4 world_pos = inv_VP * ndc;
    vec3 ray_dir = normalize(world_pos.xyz / world_pos.w);

    // Map ray direction to UV coordinates for panoramic image
    // Horizontal: based on XY angle (azimuth)
    float theta = atan(ray_dir.y, ray_dir.x);
    // Vertical: based on Z (elevation) - clamp to reasonable range
    float elevation = ray_dir.z;

    // UV mapping for panoramic background
    vec2 uv;
    uv.x = (theta + PI) / (2.0 * PI);  // 0 to 1 horizontal wrap
    // Map elevation (-1 to 1) to vertical UV, with sky at top
    uv.y = 1.0 - (elevation * 0.5 + 0.5);  // Flip so sky is at top of image

    // Sample the skybox texture
    vec4 sky_color = vec4(0.4, 0.6, 0.9, 1.0);  // Default sky blue
    if (skybox_texture != uvec2(0)) {
        sky_color = texture(sampler2D(skybox_texture), uv);
    }

    FragColor = sky_color;
}
