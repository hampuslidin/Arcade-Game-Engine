#version 410

precision highp float;

uniform sampler2D diffTexMap; // diffuse texture map
uniform bool hasDiffTexMap;   // diffuse texture map flag
uniform vec3 diffCol;         // diffuse color

in vec2 fTexCoords;

layout(location = 0) out vec4 col;

void main()
{
  col = hasDiffTexMap ? texture(diffTexMap, fTexCoords) : vec4(diffCol, 1.0);
}
