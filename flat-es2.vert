#version 100

precision mediump float;
precision mediump int;

attribute vec4 vPosition;
attribute vec3 vColor;
attribute vec3 vNormal;

uniform mat4 modelview;
uniform mat4 projection;

varying lowp vec3 Color;
varying mediump vec3 fragNormal;

void main()
{
	Color = vColor;
	fragNormal = vNormal * mat3(modelview);
	gl_Position = modelview * projection * vPosition;
}
