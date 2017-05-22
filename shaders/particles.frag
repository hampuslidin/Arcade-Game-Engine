#version 410

precision highp float;

uniform sampler2D diffuseMap;

in float fLife;

layout(location = 0) out vec4 color;

void main()
{
  color      = texture(diffuseMap, gl_PointCoord);
  color.xyz *= (1.0-fLife);
  color.w    = color.w * (1.0-pow(fLife, 4.0)) * 0.05;
}
