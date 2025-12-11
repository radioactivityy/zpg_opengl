#version 460 core

in float life;
in float depth;

out vec4 FragColor;

void main()
{
    // Discard dead particles
    if (life <= 0.0) discard;

    // Rain drop color - bright white/blue
    vec3 rain_color = vec3(0.85, 0.9, 1.0);

    // Simple circular point with soft edges
    vec2 coord = gl_PointCoord * 2.0 - 1.0;
    float dist = length(coord);

    // Soft circle falloff
    float alpha = (1.0 - dist) * life * 0.8;

    if (alpha < 0.05) discard;

    FragColor = vec4(rain_color, alpha);
}
