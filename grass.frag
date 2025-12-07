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

    // Sample texture directly - output raw color
    vec4 texColor = texture(sampler2D(mat.tex_diffuse), tex_coord);

    // Output raw texture color (should show green grass on dark background)
    FragColor = vec4(texColor.rgb, 1.0);
}
