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

    // Point size based on distance with twinkle effect for rain streaks
    float dist = max(length(pos - camera_pos), 1.0);
    float twinkle = 0.85 + 0.25 * sin(time * 12.0 + pos.x + pos.y);
    gl_PointSize = clamp((120.0 / dist) * twinkle, 3.0, 12.0);

    depth = clip_pos.z / clip_pos.w;
}
