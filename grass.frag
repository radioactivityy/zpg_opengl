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

void main(void)
{
    // Get material from SSBO
    Material mat = materials[material_index];

    // Sample grass texture
    vec4 texColor = vec4(mat.diffuse, 1.0);
    if (mat.tex_diffuse != uvec2(0)) {
        texColor = texture(sampler2D(mat.tex_diffuse), tex_coord);
    }

    // Calculate alpha from luminance - dark pixels become transparent
    float luminance = dot(texColor.rgb, vec3(0.299, 0.587, 0.114));
    float alpha = smoothstep(0.1, 0.4, luminance);

    // Discard fully transparent pixels
    if (alpha < 0.1) {
        discard;
    }

    // Simple lighting
    vec3 N = normalize(normal_ws);
    vec3 L = normalize(light_ws - position_ws);
    float NdotL = max(dot(N, L), 0.0);

    // Grass color with simple ambient + diffuse lighting
    vec3 grassColor = texColor.rgb * (0.5 + 0.5 * NdotL);

    // Boost saturation slightly for more vibrant grass
    float gray = dot(grassColor, vec3(0.299, 0.587, 0.114));
    grassColor = mix(vec3(gray), grassColor, 1.3);

    FragColor = vec4(grassColor, alpha);
}
