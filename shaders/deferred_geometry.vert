#version 410

uniform mat4 M; // model matrix
uniform mat4 V; // view matrix
uniform mat4 P; // projection matrix
uniform mat3 N; // normal matrix

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNorm;
layout(location = 2) in vec2 vTexCoords;

out vec3 fPos;
out vec3 fNorm;
out vec2 fTexCoords;

void main() 
{
	gl_Position = P * V * M * vec4(vPos, 1.0);
  fPos        = (M * vec4(vPos, 1.0)).xyz;
  fNorm       = N * vNorm;
	fTexCoords  = vTexCoords;
}
