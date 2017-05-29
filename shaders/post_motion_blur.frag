#version 410

uniform sampler2D diffTexMap;
uniform sampler2D velTexMap;
uniform sampler2D depthTexMap;
uniform mat4 currToPrev;
uniform float fps;
uniform int mode;
uniform float velScaling;
uniform bool adaptVariableFPS;
uniform bool adaptNumSamples;
uniform int prefNumSamples;

layout(location = 0) out vec4 oCol;

const float targetFPS = 60;

void main()
{
  // calculate texture coordinates
  vec2 texelSize = 1.0 / vec2(textureSize(diffTexMap, 0));
  vec2 texCoords = gl_FragCoord.xy * texelSize;
  
  vec2 vel;
  if (mode == 0) // Camera
  {
    // calculate current screen space position
    float zOverW    = texture(depthTexMap, texCoords).x;
    vec4 currentPos = vec4(2.0 * texCoords - 1.0, zOverW, 1.0);
    
    // calculate previous screen space position
    vec4 prevPos = currToPrev * currentPos;
    prevPos     /= prevPos.w;
    
    // calculate screen space velocity
    vel  = 0.5 * (currentPos.xy - prevPos.xy);
  }
  else // Per object
  {
    // sample initial velocity
    vel = texture(velTexMap, texCoords).rg;
  }
  
  // scale velocity
  vel *= velScaling;
  if (adaptVariableFPS)
    vel *= fps / targetFPS;
  
  // calculate speed and number of samples
  int numSamples;
  if (adaptNumSamples)
    numSamples = clamp(int(length(vel / texelSize)), 1, prefNumSamples);
  else
    numSamples = prefNumSamples;
  
  // blur
  oCol.rgb = texture(diffTexMap, texCoords).rgb;
  for (int i = 1; i < numSamples; ++i)
  {
    vec2 offset = vel * (float(i) / float(numSamples-1) - 0.5);
    oCol.rgb += texture(diffTexMap, texCoords + offset).rgb;
  }
  oCol.rgb /= float(numSamples);
  oCol.a    = 1.0;
}
