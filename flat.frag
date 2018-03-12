#version 330

in vec3 Color;
in vec3 fragNormal;

out vec4 outcol;

void main()
{
	vec3 lightpos = vec3(0.1, 50.1, -1.0);
	float brightness = 0.1 + (clamp(dot(fragNormal, lightpos), 0.0, 1.0) * 1.0);
	outcol = brightness * vec4(Color, 1);
}
