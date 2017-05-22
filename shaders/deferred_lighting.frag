#version 410

uniform sampler2D gCol;
uniform sampler2D gPos;
uniform sampler2D gNorm;
uniform int numLights;

in vec2 fTexCoords;

layout(location = 0) out vec4 col;

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
  vec3 fCol  = texture(gCol,  fTexCoords).rgb;
  vec3 fPos  = texture(gPos,  fTexCoords).rgb;
  vec3 fNorm = texture(gNorm, fTexCoords).rgb;
  
  vec3 lighting = fCol*0.3;
  int amount = min(max(numLights, 0), maxNumLights);
  for (int i = 0; i < amount; ++i)
  {
    Light l      = lights[i];
    vec3 v       = l.pos-fPos;
    vec3 diffuse = max(dot(fNorm, normalize(v)), 0.0) * fCol * l.col;
    float dist   = length(v);
    float att    = 1.0/(1.0 + l.lin*dist + l.quad*dist*dist);
    diffuse     *= att;
    lighting    += diffuse;
  }
  
  col = vec4(lighting, 1.0);
}
