#version 330

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec3 vColor;

out vec3 Color;

void main()
{
	Color = vColor;
	gl_Position = vPosition;
}
