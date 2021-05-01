#version 100

precision mediump int;

varying lowp vec3 Color;

void main()
{
	gl_FragColor = vec4(Color, 1.0);
}
