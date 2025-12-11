#version 460 core

in float life;
in float depth;

out vec4 FragColor;

void main()
{
    // Discard dead particles
    if (life <= 0.0) discard;

    // Rain drop color - light blue/white with transparency
    vec3 rain_color = vec3(0.7, 0.8, 0.9);

    // Fade based on life and make elongated streak
    vec2 coord = gl_PointCoord * 2.0 - 1.0;

    // Elongate vertically for rain streak effect
    float streak = exp(-coord.x * coord.x * 8.0) * exp(-coord.y * coord.y * 0.5);

    // Alpha based on life and streak shape
    float alpha = streak * life * 0.6;

    if (alpha < 0.01) discard;

    FragColor = vec4(rain_color, alpha);
}
