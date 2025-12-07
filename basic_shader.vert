#version 460 core

// vertex attributes
layout ( location = 0 ) in vec4 in_position_ms;

// uniform variables
uniform mat4 P; // Model View Projection

void main( void )
{
	gl_Position = P * in_position_ms; // model-space -> clip-space
}
