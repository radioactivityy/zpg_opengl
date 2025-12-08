#version 460 core
// Vertex attributes
layout (location = 0) in vec4 in_position_ms;
layout (location = 1) in vec3 in_normal_ms;
layout (location = 2) in vec3 in_tangent_ms;
layout (location = 3) in vec2 in_tex_coord;
layout (location = 4) in int in_mat_idx;
// Uniform variables
uniform mat4 M;   // Model matrix
uniform mat3 Mn;  // Normal matrix
uniform mat4 V;   // View matrix
uniform mat4 P;   // Projection matrix
uniform mat4 light_space_matrix;  // Light's projection * view matrix
// Outputs to fragment shader
out vec3 position_ws;
out vec3 normal_ws;
out vec3 tangent_ws;
out vec3 bitangent_ws;
out vec2 tex_coord;
out vec4 position_lcs;  // Position in light clip space for shadow mapping
flat out int material_index;
void main(void)
{
    // Transform position to world space
    vec4 pos_ws = M * in_position_ms;
    position_ws = pos_ws.xyz / pos_ws.w;

    // Transform to clip space for rasterization
    gl_Position = P * V * pos_ws;

    // Transform to light clip space for shadow mapping
    position_lcs = light_space_matrix * pos_ws;

    // Transform normal to world space
    vec3 norm_ws = Mn * in_normal_ms;
    normal_ws = normalize(norm_ws);

    // Transform tangent to world space
    vec3 tan_ws = Mn * in_tangent_ms;


    // Gram-Schmidt orthogonalization: make tangent perpendicular to normal
    tangent_ws = normalize(tan_ws - dot(tan_ws, normal_ws) * normal_ws);

    // Calculate bitangent (perpendicular to both normal and tangent)
    bitangent_ws = cross(normal_ws, tangent_ws);

    // Pass through texture coordinates and material index
    tex_coord = vec2(in_tex_coord.x, 1.0f - in_tex_coord.y);
    material_index = in_mat_idx;
}