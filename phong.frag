#version 460 core
#extension GL_ARB_bindless_texture : require
// REMOVE THIS LINE - not available on Intel IGPs:
// #extension GL_ARB_gpu_shader_int64 : require

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

    // Get diffuse color with alpha
    vec4 diffuse_rgba = vec4(mat.diffuse, 1.0);
    if (mat.tex_diffuse != uvec2(0)) {  // Check if not zero
        diffuse_rgba = texture(sampler2D(mat.tex_diffuse), tex_coord);
    }
    vec3 diffuse_color = diffuse_rgba.rgb;
    float alpha = diffuse_rgba.a;

    // Discard fully transparent fragments (alpha cutoff for grass/foliage)
    if (alpha < 0.1) {
        discard;
    }

    // Get normal
    vec3 N = normalize(normal_ws);
    /*if (mat.tex_normal != uvec2(0)) {  // Check if not zero
        vec3 T = normalize(tangent_ws);
        vec3 B = normalize(bitangent_ws);
        mat3 TBN = mat3(T, B, N);
        
        vec3 normal_ts = texture(sampler2D(mat.tex_normal), tex_coord).rgb * 2.0 - 1.0;
        N = normalize(TBN * normal_ts);
    }*/
    
    // Get RMA values
    float roughness = mat.rma.x;
    float metalness = mat.rma.y;
    float ao = mat.rma.z;
    
    if (mat.tex_rma != uvec2(0)) {  // Check if not zero
        vec3 rma_sample = texture(sampler2D(mat.tex_rma), tex_coord).rgb;
        roughness = rma_sample.r;
        metalness = rma_sample.g;
        ao = rma_sample.b;
    }
    
    // Lighting calculations
    vec3 L = normalize(light_ws - position_ws);
    vec3 V = normalize(camera_pos_ws - position_ws);
    vec3 H = normalize(L + V);
    
    // Ambient with AO
    vec3 ambient = ambient_color * diffuse_color * ao;
    
    // Diffuse
    float NdotL = max(dot(N, L), 0.0);
    vec3 diffuse = NdotL * light_color * diffuse_color;
    
    // Specular
    float shininess = (1.0 - roughness) * 128.0;
    float spec = pow(clamp(dot(N, H), 0.0, 1.0), shininess);
    vec3 F0 = mix(vec3(0.04), diffuse_color, metalness);
    vec3 specular = spec * light_color * F0;
    
    // Distance attenuation - very gentle for sun-like lighting
    float distance = length(light_ws - position_ws);
    float attenuation = 1.0 / (1.0 + 0.0001 * distance);  // Much gentler falloff

    // Final color
    vec3 result = ambient + attenuation * (diffuse + specular);

    // tone mapping
    result = result / (result + vec3(1.0));

    // Output with alpha for transparency
    FragColor = vec4(result, alpha);
}