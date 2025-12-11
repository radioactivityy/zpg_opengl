#version 460 core

// Fullscreen triangle vertex positions
const vec2 positions[3] = vec2[](
    vec2(-1.0, -1.0),
    vec2( 3.0, -1.0),
    vec2(-1.0,  3.0)
);

// Output to fragment shader
out vec2 tex_coord;

void main()
{
    vec2 pos = positions[gl_VertexID];
    gl_Position = vec4(pos, 0.9999, 1.0);
    tex_coord = pos * 0.5 + 0.5;
}
