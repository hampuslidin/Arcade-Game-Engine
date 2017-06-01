#version 410

uniform sampler2D colTexMap;

in float fLife;

layout(location = 0) out vec4 oCol;

void main()
{
  oCol      = texture(colTexMap, gl_PointCoord);
  oCol.rgb *= (1.0-fLife);
  oCol.a   *= (1.0-pow(fLife, 4.0)) * 0.05;
}
