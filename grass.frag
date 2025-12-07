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

// Uniform variables
uniform vec3 light_ws;
uniform vec3 camera_pos_ws;
uniform vec3 light_color;
uniform vec3 ambient_color;

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

    // Get normal - for grass, use geometric normal (no normal mapping)
    vec3 N = normalize(normal_ws);

    // Lighting calculations
    vec3 L = normalize(light_ws - position_ws);
    vec3 V = normalize(camera_pos_ws - position_ws);
    vec3 H = normalize(L + V);

    // Ambient - keep original grass color
    vec3 ambient = ambient_color * diffuse_color;

    // Diffuse - simpler lighting to preserve grass color
    float NdotL = max(dot(N, L), 0.0);
    vec3 diffuse = NdotL * light_color * diffuse_color * 0.8;

    // No specular for grass - keeps it matte
    vec3 specular = vec3(0.0);

    // Distance attenuation
    float distance = length(light_ws - position_ws);
    float attenuation = 1.0 / (1.0 + 0.0001 * distance);

    // Final color - boost green channel slightly for vibrant grass
    vec3 result = ambient + attenuation * diffuse;
    result = result * 1.3;  // Brighten grass

    // Tone mapping
    result = result / (result + vec3(1.0));

    // Output with alpha for transparency
    FragColor = vec4(result, alpha);
}
