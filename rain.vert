#version 460 core

// Per-particle attributes
layout (location = 0) in vec3 in_position;  // Particle position
layout (location = 1) in float in_life;      // Particle lifetime (0-1)

// Uniforms
uniform mat4 VP;  // View-Projection matrix
uniform vec3 camera_pos;
uniform float time;

// Output to fragment shader
out float life;
out float depth;

void main()
{
    life = in_life;

    // Rain drop position
    vec3 pos = in_position;

    // Transform to clip space
    vec4 clip_pos = VP * vec4(pos, 1.0);
    gl_Position = clip_pos;

    // Point size based on distance (closer = larger)
    float dist = length(pos - camera_pos);
    gl_PointSize = clamp(100.0 / dist, 2.0, 8.0);

    depth = clip_pos.z / clip_pos.w;
}
