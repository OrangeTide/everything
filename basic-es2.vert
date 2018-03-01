#version 100

precision mediump float;
precision mediump int;

attribute vec4 vPosition;
attribute vec3 vColor;

varying lowp vec3 Color;

void main()
{
	Color = vColor;
	gl_Position = vPosition;
}
