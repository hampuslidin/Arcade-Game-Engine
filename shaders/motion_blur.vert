#version 410

layout(location = 0) in vec2 vPos;

out vec2 fTexCoords;

void main()
{
  gl_Position = vec4(vPos, 0.0, 1.0);
  fTexCoords  = 0.5*(vPos + 1.0);
}
