#version 410

uniform mat4 modelViewProjectionMatrix;

layout(location = 0) in vec3 vPosition;

void main()
{
  gl_Position = modelViewProjectionMatrix * vec4(vPosition, 1.0);
}
