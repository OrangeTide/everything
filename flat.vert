#version 330
#extension GL_ARB_explicit_attrib_location : require

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vColor;
layout(location = 2) in vec3 vNormal;

out vec3 Color;
out vec3 fragNormal;

uniform mat4 modelview;
uniform mat4 projection;

void main()
{
	Color = vColor;
	fragNormal = vNormal * mat3(modelview);
	gl_Position = projection * modelview * vec4(vPosition, 1.0);
}
