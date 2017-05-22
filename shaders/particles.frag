#version 410

precision highp float;

uniform sampler2D diffTexMap;

in float fLife;

layout(location = 0) out vec4 col;

void main()
{
  col      = texture(diffTexMap, gl_PointCoord);
  col.xyz *= (1.0-fLife);
  col.w    = col.w * (1.0-pow(fLife, 4.0)) * 0.05;
}
