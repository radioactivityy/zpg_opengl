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
uniform float time;  // Time for animation

// Outputs to fragment shader
out vec3 position_ws;
out vec3 normal_ws;
out vec3 tangent_ws;
out vec3 bitangent_ws;
out vec2 tex_coord;
flat out int material_index;

// Random function for grass variation
float rand(const vec2 co) {
    return fract(sin(dot(co, vec2(12.9898f, 78.233f))) * 43758.5453f);
}

void main(void)
{
    // Get world position first
    vec4 pos_ws = M * in_position_ms;

    // Wind animation - apply more movement to top of grass (higher vertices)
    // Use vertex height (z or y depending on model orientation) to determine sway amount
    float height_factor = in_position_ms.z;  // Higher = more sway
    height_factor = max(0.0, height_factor);  // Only positive heights sway

    // Create wind effect using sin/cos with time and position-based variation
    vec2 wind_dir = vec2(1.0, 0.5);  // Wind direction
    float wind_strength = 0.3;  // How much the grass moves

    // Add randomness based on world position for natural variation
    float random_offset = rand(pos_ws.xy) * 6.28318;  // Random phase offset
    float wind_wave = sin(time * 2.0 + pos_ws.x * 0.5 + pos_ws.y * 0.3 + random_offset);

    // Apply wind displacement (only to x and y, not z)
    pos_ws.x += wind_wave * wind_strength * height_factor * wind_dir.x;
    pos_ws.y += wind_wave * wind_strength * height_factor * wind_dir.y;

    position_ws = pos_ws.xyz / pos_ws.w;

    // Transform to clip space for rasterization
    gl_Position = P * V * pos_ws;

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
