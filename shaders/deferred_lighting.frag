#version 410

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gColor;

in vec2 fTextureCoordinates;

layout(location = 0) out vec4 color;

struct Light
{
  vec3 position;
  vec3 color;
  float linear;
  float quadratic;
};
const int numberOfLights = 36;
uniform Light lights[numberOfLights];

void main()
{
  vec3 fPosition = texture(gPosition, fTextureCoordinates).rgb;
  vec3 fNormal   = texture(gNormal,   fTextureCoordinates).rgb;
  vec3 fColor    = texture(gColor,    fTextureCoordinates).rgb;
  
  vec3 lighting = fColor*0.3;
  for (int i = 0; i < numberOfLights; ++i)
  {
    Light l = lights[i];
    vec3 lightVector = l.position-fPosition;
    vec3 lightDir    = normalize(lightVector);
    vec3 diffuse     = max(dot(fNormal, lightDir), 0.0) * fColor * l.color;
    float dist       = length(lightVector);
    float att        = 1.0/(1.0+l.linear*dist+l.quadratic*dist*dist);
    diffuse  *= att;
    lighting += diffuse;
  }
  
  color = vec4(lighting, 1.0);
}
