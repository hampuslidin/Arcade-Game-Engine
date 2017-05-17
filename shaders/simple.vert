#version 410

uniform mat4 modelViewProjectionMatrix;

layout(location = 0) in vec3 vPosition;
layout(location = 2) in vec2 vTextureCoordinates;

out vec2 fTextureCoordinates;

void main() 
{
	gl_Position = modelViewProjectionMatrix * vec4(vPosition, 1.0);
	fTextureCoordinates = vTextureCoordinates;
}
