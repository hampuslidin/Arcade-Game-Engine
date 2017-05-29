#version 410

uniform sampler2D diffTexMap;

layout(location = 0) out vec4 oCol;

void main()
{
  vec2 texCoords = gl_FragCoord.xy / vec2(textureSize(diffTexMap, 0));
  oCol           = texture(diffTexMap, texCoords);
}
