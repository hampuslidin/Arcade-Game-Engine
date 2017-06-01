//
//  particles.cpp
//  Game Engine
//

#include <stb_image.h>

#include "core.hpp"


// MARK: -
// MARK: Properties
int ParticleSystemComponent::numberOfParticles() const
{
  return (int)_particles.size();
}

// MARK: Member functions
ParticleSystemComponent::ParticleSystemComponent(int maxNumberOfParticles)
  : Component()
  , _maxNumberOfParticles(maxNumberOfParticles)
{}

void ParticleSystemComponent::init(Entity * entity)
{
  Component::init(entity);
  
  // generate vertex array object
  glGenVertexArrays(1, &_vertexArrayObject);
  glBindVertexArray(_vertexArrayObject);
  
  // generate particle data buffer
  glGenBuffers(1, &_particleDataBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, _particleDataBuffer);
  glBufferData(GL_ARRAY_BUFFER, 4*_maxNumberOfParticles*sizeof(float), nullptr,
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 4, GL_FLOAT, false, 0, 0);
  glEnableVertexAttribArray(0);
}

void ParticleSystemComponent::render(const Core & core)
{
  // spawn new particles
  const mat4 model = entity()->worldTranslation() * entity()->worldRotation();
  const int amount = std::min(_maxNumberOfParticles-(int)_particles.size(),
                              core.particleSpawnRate());
  for (int i = 0; i < amount; ++i)
  {
    const float theta = Core::uniformRandom(0.0f, 2.0f*(float)M_PI);
    const float u     = Core::uniformRandom(1.0f-core.particleConeSize(), 1.0f);
    const float c     = sqrt(1.0f - u*u);
    const vec4 p = model * vec4(0.0f, 0.0f, 0.0f, 1.0f);
    const vec3 v = vec3(model * vec4(u, c*cosf(theta), c*sinf(theta), 0.0f));
    _particles.push_back
    ({
      0.0f,
      core.particleLifeTime(),
      vec3(p)/p.w,
      core.particleVelocity()*v});
  }
  
  // remove dead particles
  for (int id = 0; id < _particles.size(); ++id)
  {
    auto & particle = _particles[id];
    if (particle.age >= particle.lifeTime)
    {
      // kill particle
      int lastId = (int)_particles.size()-1;
      if (id < lastId)
        particle = _particles[lastId];
      _particles.pop_back();
    }
  }
  
  // update alive particles
  for (auto & particle : _particles)
  {
    float dt           = (float)core.deltaTime();
    particle.age      += dt;
    particle.position += particle.velocity * dt;
  }
  
  // extract particle render data
  const mat4 view = core.viewMatrix();
  int numberOfParticles = (int)_particles.size();
  _particleRenderData.resize(numberOfParticles);
  for (int i = 0; i < numberOfParticles; ++i)
  {
    auto & particle           = _particles[i];
    auto & particleRenderData = _particleRenderData[i];
    const vec4 p              = view * vec4(particle.position, 1.0f);
    particleRenderData.x      = p.x / p.w;
    particleRenderData.y      = p.y / p.w;
    particleRenderData.z      = p.z / p.w;
    particleRenderData.w      = particle.age / particle.lifeTime;
  }
  
  // sort particle render data
  const auto sorter = [](const vec4 & a, const vec4 & b) { return a.z < b.z; };
  const auto startIterator = _particleRenderData.begin();
  const auto endIterator   = next(startIterator, numberOfParticles);
  sort(startIterator, endIterator, sorter);
  
  // upload particle render data
  glBindBuffer(GL_ARRAY_BUFFER, _particleDataBuffer);
  glBufferSubData(GL_ARRAY_BUFFER, 0, 4*numberOfParticles*sizeof(float),
                  &_particleRenderData[0].x);
  
  // set shader uniforms
  const mat4 projection = core.projectionMatrix();
  const ivec2 d         = core.viewDimensions();
  glUseProgram(_shaderProgram);
  glUniformMatrix4fv(_projMatrixLocation, 1, false, &projection[0].x);
  glUniform1f(_screenWidthLocation, (float)d.x);
  glUniform1f(_screenHeightLocation, (float)d.y);
  
  // activate color map
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, _diffuseMap);
  
  // render
  glBindVertexArray(_vertexArrayObject);
  glDrawArrays(GL_POINTS, 0, numberOfParticles);
  glUseProgram(0);
}

bool ParticleSystemComponent::loadTexture(const char * textureFileName)
{
  if (_shaderProgram)
  {
    // load image
    int width, height, components;
    auto image = stbi_load(textureFileName, &width, &height, &components,
                           STBI_rgb_alpha);
    if (image) {
      // generate texture
      glGenTextures(1, &_diffuseMap);
      glBindTexture(GL_TEXTURE_2D, _diffuseMap);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                   GL_UNSIGNED_BYTE, image);
      free(image);
      
      // set filtering options and attach texture to shader
      glUseProgram(_shaderProgram);
      glGenerateMipmap(GL_TEXTURE_2D);
      
      return true;
    }
  }
  return false;
}

bool ParticleSystemComponent::loadShader(const char * vertexShaderFileName,
                                         const char * fragmentShaderFileName)
{
  auto result = Core::CreateShaderProgram(vertexShaderFileName,
                                          fragmentShaderFileName);
  if (!result.isNothing())
  {
    _shaderProgram = result;
    
    // retrieve uniform locations
    _projMatrixLocation =
      glGetUniformLocation(_shaderProgram, "projectionMatrix");
    _screenWidthLocation =
      glGetUniformLocation(_shaderProgram, "screenWidth");
    _screenHeightLocation =
      glGetUniformLocation(_shaderProgram, "screenHeight");
    return true;
  }
  return false;
}


string ParticleSystemComponent::trait() const
{
  return "ParticleSystem";
}
