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
    // Fullscreen triangle using vertex ID
    vec2 pos = positions[gl_VertexID];

    // Output position (z = 1.0 for far plane, will be rendered behind everything)
    gl_Position = vec4(pos, 0.9999, 1.0);

    // Texture coordinates for the quad portion (0-1 range)
    tex_coord = pos * 0.5 + 0.5;
}
