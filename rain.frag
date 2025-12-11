#version 460 core

in float life;
in float depth;

out vec4 FragColor;

void main()
{
    // Discard dead particles
    if (life <= 0.0) discard;

   vec3 rain_color = vec3( 1.0);

    // Simple circular point with soft edges
    vec2 coord = gl_PointCoord * 2.0 - 1.0;
    float dist = length(coord);


    float alpha = smoothstep(1.0, 0.0, dist) * life * 0.9 ; 

    if (alpha < 0.05) discard;

    FragColor = vec4(rain_color, alpha);
}
