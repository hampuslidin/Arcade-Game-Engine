#version 410

precision highp float;

in vec3 outColor;
layout(location = 0) out vec4 fragmentColor;

void main() 
{
	fragmentColor.rgb = outColor;
}
