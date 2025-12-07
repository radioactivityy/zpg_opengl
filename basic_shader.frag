#version 460 core
#extension GL_ARB_bindless_texture : require
#extension GL_ARB_gpu_shader_int64 : require

// Inputs from vertex shader
in vec3 position_ws;
in vec3 normal_ws;
in vec3 tangent_ws;
in vec3 bitangent_ws;
in vec2 tex_coord;
flat in uint material_index;

// Outputs
layout (location = 0) out vec4 FragColor;

struct Material {
    vec3 diffuse;
    uint64_t tex_diffuse;
    
    vec3 rma;  // roughness, metalness, ao
    uint64_t tex_rma;
    
    vec3 normal;
    uint64_t tex_normal;
};

// SSBO for materials
layout(std430, binding = 0) readonly buffer Materials {
    Material materials[];
};

//Todo-ssbo and tbn in vert normal_ts in frag shader and then normal-> tbn * normal_ts -- normalize(texture(sampler2D(materials[mat_idx].tex-normal
// Uniform variables - ADD THESE
uniform vec3 light_ws;
uniform vec3 camera_pos_ws;
uniform vec3 light_color;
uniform vec3 ambient_color;
uniform vec3 material_diffuse;
uniform vec3 material_specular;
uniform float material_shininess;

void main(void)
{
    // Normalize the interpolated normal
    vec3 N = normalize(normal_ws);
    
    // Light direction (from fragment to light)
    vec3 L = normalize(light_ws - position_ws);
    
    // View direction (from fragment to camera)
    vec3 V = normalize(camera_pos_ws - position_ws);
    
    // Ambient component
    vec3 ambient = ambient_color * material_diffuse;
    
    // Diffuse component (Lambertian)
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * light_color * material_diffuse;
    
    // Specular component (Blinn-Phong)
    vec3 H = normalize(L + V);
    float spec = pow(clamp(dot(N, H), 0.0, 1.0), material_shininess);
    vec3 specular = spec * light_color * material_specular;
    
    // Distance attenuation
    float distance = length(light_ws - position_ws);
    float attenuation = 1.0 / (1.0 + 0.001 * distance + 0.0001 * distance * distance);
    
    // Combine all components
    vec3 result = ambient + attenuation * (diffuse + specular);
    
    FragColor = vec4(result, 1.0);
}