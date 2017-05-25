#version 410

precision highp float;

uniform sampler2D diffTexMap;

in float fLife;

layout(location = 0) out vec4 oCol;

void main()
{
  oCol      = texture(diffTexMap, gl_PointCoord);
  oCol.xyz *= (1.0-fLife);
  oCol.w    = oCol.w * (1.0-pow(fLife, 4.0)) * 0.05;
}
