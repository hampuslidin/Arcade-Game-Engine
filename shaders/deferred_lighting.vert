#version 410

layout(location = 0) in vec2 vPosition;

out vec2 fTextureCoordinates;

void main()
{
  gl_Position         = vec4(vPosition, 0.0, 1.0);
  fTextureCoordinates = 0.5*(vPosition + 1.0);
}

