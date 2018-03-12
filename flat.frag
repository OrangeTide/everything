#version 330

in vec3 Color;

out vec4 outcol;

void main()
{
	outcol = vec4(Color, 1);
}
