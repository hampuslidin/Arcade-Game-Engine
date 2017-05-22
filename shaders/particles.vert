#version 410

uniform mat4 projectionMatrix;
uniform float screenWidth;
uniform float screenHeight;

layout(location = 0) in vec4 vParticle;

out float fLife;

void main()
{
  fLife             = vParticle.w;
  vec4 position     = vec4(vParticle.xyz, 1.0);
  vec4 quad         = projectionMatrix * vec4(1.0, 1.0, position.z, position.w);
  vec2 pixelSize    = vec2(screenWidth, screenHeight) * quad.xy / quad.w;
  float scaleFactor = pixelSize.x + pixelSize.y;
  gl_Position       = projectionMatrix * position;
  gl_PointSize      = scaleFactor * mix(0.0, 0.5, pow(fLife, 0.25));
}
