#version 330
#extension GL_ARB_explicit_attrib_location : require

layout(location = 0) in vec4 vPosition;
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
	gl_Position = modelview * projection * vPosition;
}
