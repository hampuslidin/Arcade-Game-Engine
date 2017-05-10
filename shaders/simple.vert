#version 410

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 textureCoordinates;
out vec2 outTextureCoordinates;
uniform mat4 modelViewProjectionMatrix;

void main() 
{
	gl_Position = modelViewProjectionMatrix * vec4(position, 1.0);
	outTextureCoordinates = textureCoordinates;
}
