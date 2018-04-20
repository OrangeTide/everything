#version 330

in vec2 tex;

out vec4 outcol;

uniform sampler2D texsampler;

void main()
{
	outcol = texture(texsampler, tex);
}
