#version 410

uniform mat4 projectionMatrix;
uniform float screenWidth;
uniform float screenHeight;

layout(location = 0) in vec4 vParticle;

out float fLife;

void main()
{
  // TODO: proper scaling
  fLife             = vParticle.w;
  gl_Position       = projectionMatrix * vec4(vParticle.xyz, 1.0);
  gl_PointSize      = 1.0;
  gl_PointSize      = 10.0 * mix(0.0, 5.0, pow(fLife, 0.25));
}
