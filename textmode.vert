#version 330
#extension GL_ARB_explicit_attrib_location : require

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec2 texCoord;

out vec2 tex;

uniform mat4 modelview;
uniform mat4 projection;

void main()
{
	tex = texCoord;
	gl_Position = projection * modelview * vec4(vPosition, 1.0);
}
