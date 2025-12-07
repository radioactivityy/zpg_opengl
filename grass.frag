#version 460 core
#extension GL_ARB_bindless_texture : require

// Inputs from vertex shader
in vec3 position_ws;
in vec3 normal_ws;
in vec3 tangent_ws;
in vec3 bitangent_ws;
in vec2 tex_coord;
flat in int material_index;

// Outputs
layout (location = 0) out vec4 FragColor;

struct Material {
    vec3 diffuse;
    uvec2 tex_diffuse;
    vec3 rma;
    uvec2 tex_rma;
    vec3 normal;
    uvec2 tex_normal;
};

// SSBO for materials
layout(std430, binding = 0) readonly buffer Materials {
    Material materials[];
};

// Uniform variables (only what we need for grass)
uniform vec3 light_ws;
uniform vec3 camera_pos_ws;

void main(void)
{
    // Get material from SSBO
    Material mat = materials[material_index];

    // Get diffuse color
    vec4 diffuse_rgba = vec4(mat.diffuse, 1.0);
    if (mat.tex_diffuse != uvec2(0)) {
        diffuse_rgba = texture(sampler2D(mat.tex_diffuse), tex_coord);
    }
    vec3 diffuse_color = diffuse_rgba.rgb;

    // Generate alpha from luminance for grass (RGB textures without alpha)
    // Dark pixels become transparent
    float luminance = dot(diffuse_color, vec3(0.299, 0.587, 0.114));

    // More aggressive threshold - grass textures often have dark (not black) backgrounds
    // Pixels with luminance below 0.2 become transparent, soft edge up to 0.4
    float alpha = smoothstep(0.15, 0.35, luminance);

    // Discard fully transparent fragments
    if (alpha < 0.1) {
        discard;
    }

    // Simple grass rendering - just show the texture color directly
    // No complex lighting to preserve the original green color
    vec3 result = diffuse_color * 1.2;  // Slight brightness boost

    // Output with alpha for transparency
    FragColor = vec4(result, alpha);
}
