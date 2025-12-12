#version 460 core

in float life;
in float depth;

out vec4 FragColor;

void main()
{
    // Discard dead particles
    if (life <= 0.0) discard;

    // Rain drop color - bright white
    vec3 rain_color = vec3(1.0);

    // Create elongated vertical streak (rain drop shape)
    vec2 coord = gl_PointCoord * 2.0 - 1.0;

    // Stretch vertically to make rain streaks
    float streak_x = abs(coord.x) * 3.0;  // Narrow horizontally
    float streak_y = abs(coord.y) * 0.3;  // Stretch vertically

    // Combined distance for elongated shape
    float dist = streak_x * streak_x + streak_y * streak_y;

    // Soft falloff for streak
    float alpha = exp(-dist * 2.0) * life * 0.9;

    if (alpha < 0.1) discard;

    FragColor = vec4(rain_color, alpha);
}
