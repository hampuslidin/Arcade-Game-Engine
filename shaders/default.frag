#version 410

precision highp float;

uniform sampler2D diffTexMap; // diffuse texture map
uniform bool hasDiffTexMap;   // diffuse texture map flag
uniform vec3 diffCol;         // diffuse color

in vec2 fTexCoords;
in vec4 fPVMPos;
in vec4 fPrevPVMPos;

layout(location = 0) out vec4 oCol;
layout(location = 1) out vec2 oVel;

void main()
{
  oCol = hasDiffTexMap ? texture(diffTexMap, fTexCoords) : vec4(diffCol, 1.0);
  oVel = 0.5 * (fPVMPos.xy / fPVMPos.w - fPrevPVMPos.xy / fPrevPVMPos.w);
}
