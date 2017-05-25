#version 410

uniform sampler2D posTexMap;
uniform sampler2D normTexMap;
uniform sampler2D diffTexMap;
uniform int numLights;

in vec2 fTexCoords;

layout(location = 0) out vec4 oCol;

struct Light
{
  vec3 pos;
  vec3 col;
  float lin;
  float quad;
};
const int maxNumLights = 36;
uniform Light lights[maxNumLights];

void main()
{
  vec3 pos  = texture(posTexMap,  fTexCoords).rgb;
  vec3 norm = texture(normTexMap, fTexCoords).rgb;
  vec3 col  = texture(diffTexMap, fTexCoords).rgb;
  
  vec3 lighting = col*0.3;
  int amount = min(max(numLights, 0), maxNumLights);
  for (int i = 0; i < amount; ++i)
  {
    Light l      = lights[i];
    vec3 v       = l.pos-pos;
    vec3 diffuse = max(dot(norm, normalize(v)), 0.0) * col * l.col;
    float dist   = length(v);
    float att    = 1.0/(1.0 + l.lin*dist + l.quad*dist*dist);
    diffuse     *= att;
    lighting    += diffuse;
  }
  
  oCol = vec4(lighting, 1.0);
}
