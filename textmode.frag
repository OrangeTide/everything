#version 330

in vec2 tex;

layout(location = 0) out vec4 outcol;

uniform sampler2D texsampler;

void main()
{
	outcol = texture2D(texsampler, tex);
}
