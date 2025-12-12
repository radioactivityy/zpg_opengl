#version 460 core

// Vertex attributes
layout (location = 0) in vec4 in_position_ms;

// Uniform variables
uniform mat4 mlp;  // Model-Light-Projection matrix

void main(void)
{
    gl_Position = mlp * in_position_ms;
}
