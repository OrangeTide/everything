#version 100

precision mediump float;
precision mediump int;

attribute vec4 vPosition;
attribute vec3 vColor;

uniform mat4 MVP;

varying lowp vec3 Color;

void main()
{
	Color = vColor;
	gl_Position = MVP * vPosition;
}
