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
    // Dark/black pixels become transparent
    float luminance = dot(diffuse_color, vec3(0.299, 0.587, 0.114));

    // Use smoothstep for soft edges - pixels darker than 0.15 become transparent
    float alpha = smoothstep(0.1, 0.3, luminance);

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

    // Ambient - boost for grass visibility
    vec3 ambient = ambient_color * diffuse_color * 1.2;

    // Diffuse - wrap lighting for softer look on grass
    float NdotL = max(dot(N, L), 0.0);
    float wrap = 0.5;
    float wrappedNdotL = max((NdotL + wrap) / (1.0 + wrap), 0.0);
    vec3 diffuse = wrappedNdotL * light_color * diffuse_color;

    // Minimal specular for grass
    float shininess = 8.0;
    float spec = pow(clamp(dot(N, H), 0.0, 1.0), shininess) * 0.1;
    vec3 specular = spec * light_color * vec3(0.04);

    // Distance attenuation
    float distance = length(light_ws - position_ws);
    float attenuation = 1.0 / (1.0 + 0.0001 * distance);

    // Final color
    vec3 result = ambient + attenuation * (diffuse + specular);

    // Tone mapping
    result = result / (result + vec3(1.0));

    // Output with alpha for transparency
    FragColor = vec4(result, alpha);
}
