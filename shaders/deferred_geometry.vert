#version 410

uniform mat4 modelMatrix;
uniform mat4 modelViewProjectionMatrix;
uniform mat3 normalMatrix;

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTextureCoordinates;

out vec3 fPosition;
out vec3 fNormal;
out vec2 fTextureCoordinates;

void main() 
{
	gl_Position         = modelViewProjectionMatrix * vec4(vPosition, 1.0);
  fPosition           = (modelMatrix * vec4(vPosition, 1.0)).xyz;
  fNormal             = normalMatrix * vNormal;
	fTextureCoordinates = vTextureCoordinates;
}
