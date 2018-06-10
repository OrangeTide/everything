#version 100

precision mediump int;

varying lowp vec3 Color;
varying mediump vec3 fragNormal;

void main()
{
	mediump vec3 lightpos = vec3(0.1, 50.1, -1.0);
	lowp float brightness = 0.1 + (clamp(dot(fragNormal, lightpos), 0.0, 1.0) * 1.0);
	gl_FragColor = brightness * vec4(Color, 1.0);
}
