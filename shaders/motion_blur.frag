#version 410

uniform sampler2D diffTexMap;
uniform sampler2D depthTexMap;
uniform mat4 currToPrev;
uniform float fps;

in vec2 fTexCoords;

layout(location = 0) out vec4 oCol;

const int numSamples  = 16;
const float scaling   = 0.5;
const float targetFPS = 60;

void main()
{
  // calculate current screen space position
  float zOverW    = texture(depthTexMap, fTexCoords).x;
  vec4 currentPos = vec4(2.0 * fTexCoords - 1.0, zOverW, 1.0);
  
  // calculate previous screen space position
  vec4 prevPos = currToPrev * currentPos;
  prevPos     /= prevPos.w;
  
  // calculate screen space velocity
  vec2 vel = (currentPos.xy - prevPos.xy) / 2.0;
  vel     *= scaling * fps / targetFPS;
  
  // set initial color
  oCol = texture(diffTexMap, fTexCoords);
  
  // blur
  vec2 texCoords = fTexCoords - numSamples / 2 * vel;
  for (int i = 1; i < numSamples; ++i, texCoords += vel)
    oCol += texture(diffTexMap, texCoords);
  oCol /= numSamples;
}
