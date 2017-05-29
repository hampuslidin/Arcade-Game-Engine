#version 410

uniform sampler2D posTexMap;
uniform sampler2D normTexMap;
uniform sampler2D diffTexMap;
uniform int showQuad;
uniform vec3 lightPos;
uniform vec3 lightCol;
uniform float attDist;
uniform float attLin;
uniform float attQuad;

layout(location = 0) out vec4 oCol;

void main()
{
  // sample data from geometry pass
  vec2 texCoords = gl_FragCoord.xy / vec2(textureSize(diffTexMap, 0));
  vec3 pos       = texture(posTexMap,  texCoords).rgb;
  vec3 norm      = texture(normTexMap, texCoords).rgb;
  vec3 col       = texture(diffTexMap, texCoords).rgb;
  
  // calculate distance to light source
  vec3 lightDir = lightPos-pos;
  float dist    = length(lightDir);
  
  // perform lighting calculations if fragment is within light volume
  vec3 lighting = vec3(0.05*showQuad);
  if (dist <= attDist)
  {
    vec3 diffuse = max(dot(norm, normalize(lightDir)), 0.0) * col * lightCol;
    float att    = 1.0/(1.0 + attLin*dist + attQuad*dist*dist);
    diffuse     *= att;
    lighting    += diffuse;
  }
  oCol = vec4(lighting, 1.0);
}
