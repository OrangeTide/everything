#version 330

layout(location = 0) in vec4 vPosition;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
	gl_Position = projection * view * model * vPosition;
}
