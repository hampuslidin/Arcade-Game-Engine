#version 410

uniform sampler2D colTexMap;

layout(location = 0) out vec4 oCol;

void main()
{
  vec2 texCoords = gl_FragCoord.xy / vec2(textureSize(colTexMap, 0));
  oCol           = texture(colTexMap, texCoords);
}
