//
//  core.cpp
//  Game Engine
//

#include "core.hpp"
#include <iostream>
#include <fstream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

using namespace tinyobj;


// MARK: Member functions
Sprite::Sprite(SDL_Renderer * renderer, SDL_Texture * texture)
  : _renderer(renderer)
  , _texture(texture)
{}

Sprite * Sprite::createSprite(SDL_Renderer * renderer, const char * filename)
{
  SDL_Surface * loadedSurface = IMG_Load(filename);
  if (!loadedSurface) SDL_Log("IMG_Load: %s\n", IMG_GetError());
  else
  {
    auto texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
    SDL_FreeSurface(loadedSurface);
    return new Sprite(renderer, texture);
  }
  return nullptr;
}

void Sprite::destroy()
{
  SDL_DestroyTexture(_texture);
}

void Sprite::draw(int x, int y, int w, int h, int scale)
{
  SDL_Rect rect {x*scale, y*scale, w*scale, h*scale};
  SDL_RenderCopy(_renderer, _texture, nullptr, &rect);
}


// MARK: -
// MARK: Member functions
SpriteCollection & SpriteCollection::main()
{
  static SpriteCollection instance;
  return instance;
}

void SpriteCollection::init(SDL_Renderer * renderer)
{
  _renderer = renderer;
}

Sprite * SpriteCollection::create(string id, const char * filename)
{
  return _sprites[id] = Sprite::createSprite(_renderer, filename);
}

void SpriteCollection::destroy(string id)
{
  auto it = _sprites.find(id);
  if (it != _sprites.end())
  {
    _sprites.at(id)->destroy();
    _sprites.erase(it);
  }
}

void SpriteCollection::destroyAll()
{
  for (auto pair : _sprites)
  {
    pair.second->destroy();
  }
  _sprites.clear();
}

Sprite * SpriteCollection::retrieve(string id)
{
  if (_sprites.find(id) != _sprites.end())
    return _sprites.at(id);
  return nullptr;
}

void SpriteCollection::draw(string id, int x, int y, int w, int h, int scale)
{
  Sprite * sprite;
  if ((sprite = retrieve(id))) sprite->draw(x, y, w, h, scale);
}


// MARK: -
// MARK: Member functions
void NotificationCenter::notify(Event event, GameObject & sender)
{
  for (auto pair : _instance()._blocks[event])
  {
    if (pair.second == nullptr || pair.second == &sender) pair.first(event);
  }
}

ObserverID NotificationCenter::observe(function<void(Event)> block,
                                       Event event,
                                       const GameObject * sender)
{
  auto & blocksForEvent = _instance()._blocks[event];
  auto size = blocksForEvent.size();
  blocksForEvent.push_back({block, sender});
  return hash<string>{}(event.id() + to_string(size));
}

void NotificationCenter::unobserve(ObserverID id,
                                   Event event,
                                   const GameObject * sender)
{
  // TODO: fix so that elements are not erased, but the spot it occupied is made
  // available for the next observer. This function is currently not in use.
  auto blocks_for_event = _instance()._blocks[event];
  for (auto i = 0; i < blocks_for_event.size(); i++)
  {
    auto sender_for_block = blocks_for_event[i].second;
    if (sender_for_block == nullptr || sender == nullptr ||
        sender_for_block == sender)
    {
      size_t h = hash<string>{}(event.id() + to_string(i));
      if (h == id)
      {
        _instance()._blocks[event].erase(_instance()._blocks[event].begin()+i);
        return;
      }
    }
  }
}

// MARK: Private
NotificationCenter & NotificationCenter::_instance()
{
  static NotificationCenter instance;
  return instance;
}


// MARK: -
// MARK: Properties
const string & GameObject::id() const { return _id; }

// MARK: Member functions
void GameObject::assignIdentifier(const string & id)
{
  _id = id;
}


// MARK: -
// MARK: Member functions
void Component::init(Entity * entity)
{
  _entity = entity;
}

string Component::id()
{
  string id = trait() + "Component";
  if (_entity)
    return _entity->id() + id;
  return id;
}

// MARK: Protected
Entity * Component::entity() { return _entity; }


// MARK: -
// MARK: Member functions
string InputComponent::trait() const { return "Input"; }


// MARK: -
// MARK: Properties
const vector<float> & GraphicsComponent::vertices() const
{
  return _vertices;
}
long GraphicsComponent::numberOfVertices() const
{
  return _numberOfVertices;
}
const vec3 & GraphicsComponent::diffuseColor() const
{
  return _diffuseColor;
}

// MARK: Member functions
GraphicsComponent::GraphicsComponent()
  : Component()
  , _diffuseColor(0.8, 0.8, 0.8)
{}

void GraphicsComponent::init(Entity * entity)
{
  Component::init(entity);
  
  glGenVertexArrays(1, &_vertexArrayObject);
}

void GraphicsComponent::render(const Core & core)
{
  // construct transforms
  const mat4 view                = core.viewMatrix();
  const mat4 projection          = core.projectionMatrix();
  const mat4 model               = entity()->worldTransform();
  const mat4 modelViewProjection = projection * view * model;
  const mat3 normal              = glm::inverse(mat3(model)); // FIXME: why does this work? (should be inverse-transpose)
  
  // set shader uniforms
  glUseProgram(_shaderProgram);
  glUniformMatrix4fv(_modelMatrixLocation, 1, false, &model[0].x);
  glUniformMatrix4fv(_modelViewProjectionMatrixLocation, 1, false,
                     &modelViewProjection[0].x);
  glUniformMatrix3fv(_normalMatrixLocation, 1, true, &normal[0].x);
  glUniform3fv(_diffuseColorLocation, 1, &_diffuseColor.x);
  
  // activate textures
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, _diffuseMap);
  
  // render
  glBindVertexArray(_vertexArrayObject);
  glDrawArrays(GL_TRIANGLES, 0, (int)_numberOfVertices);
  glUseProgram(0);
}

bool GraphicsComponent::loadObject(const char * objectFileName)
{
  attrib_t           attrib;
  vector<shape_t>    shapes;
  string             error;
  LoadObj(&attrib, &shapes, nullptr, &error, objectFileName);

  if (shapes.size() >= 1 && !attrib.vertices.empty())
  {
    // initialize data collections
    vector<float> normals, textureCoordinates;
    auto shape = shapes[0];
    _numberOfVertices = shape.mesh.indices.size();
    _vertices.resize(3*_numberOfVertices);
    normals.resize(3*_numberOfVertices);
    textureCoordinates.resize(2*_numberOfVertices);
    
    // populate data collections
    for (int face = 0; face < _numberOfVertices/3; ++face)
    {
      // indices
      const auto i0 = shape.mesh.indices[3*face];
      const auto i1 = shape.mesh.indices[3*face+1];
      const auto i2 = shape.mesh.indices[3*face+2];
      
      // vertices
      const vec3 v0 =
      {
        attrib.vertices[3*i0.vertex_index],
        attrib.vertices[3*i0.vertex_index+1],
        attrib.vertices[3*i0.vertex_index+2]
      };
      const vec3 v1 =
      {
        attrib.vertices[3*i1.vertex_index],
        attrib.vertices[3*i1.vertex_index+1],
        attrib.vertices[3*i1.vertex_index+2]
      };
      const vec3 v2 =
      {
        attrib.vertices[3*i2.vertex_index],
        attrib.vertices[3*i2.vertex_index+1],
        attrib.vertices[3*i2.vertex_index+2]
      };
      
      _vertices[9*face]   = v0.x;
      _vertices[9*face+1] = v0.y;
      _vertices[9*face+2] = v0.z;
      _vertices[9*face+3] = v1.x;
      _vertices[9*face+4] = v1.y;
      _vertices[9*face+5] = v1.z;
      _vertices[9*face+6] = v2.x;
      _vertices[9*face+7] = v2.y;
      _vertices[9*face+8] = v2.z;
      
      // normals
      if (!attrib.normals.empty())
      {
        normals[9*face]   = attrib.normals[3*i0.normal_index];
        normals[9*face+1] = attrib.normals[3*i0.normal_index+1];
        normals[9*face+2] = attrib.normals[3*i0.normal_index+2];
        normals[9*face+3] = attrib.normals[3*i1.normal_index];
        normals[9*face+4] = attrib.normals[3*i1.normal_index+1];
        normals[9*face+5] = attrib.normals[3*i1.normal_index+2];
        normals[9*face+6] = attrib.normals[3*i2.normal_index];
        normals[9*face+7] = attrib.normals[3*i2.normal_index+1];
        normals[9*face+8] = attrib.normals[3*i2.normal_index+2];
      }
      else
      {
        // calculate face normal
        vec3 e0 = normalize(v1-v0);
        vec3 e1 = normalize(v2-v0);
        vec3 n  = cross(e0, e1);
        normals[9*face]   = n.x;
        normals[9*face+1] = n.y;
        normals[9*face+2] = n.z;
        normals[9*face+3] = n.x;
        normals[9*face+4] = n.y;
        normals[9*face+5] = n.z;
        normals[9*face+6] = n.x;
        normals[9*face+7] = n.y;
        normals[9*face+8] = n.z;
      }
      
      // texture coordinates
      textureCoordinates[6*face]   = attrib.texcoords[2*i0.texcoord_index];
      textureCoordinates[6*face+1] = attrib.texcoords[2*i0.texcoord_index+1];
      textureCoordinates[6*face+2] = attrib.texcoords[2*i1.texcoord_index];
      textureCoordinates[6*face+3] = attrib.texcoords[2*i1.texcoord_index+1];
      textureCoordinates[6*face+4] = attrib.texcoords[2*i2.texcoord_index];
      textureCoordinates[6*face+5] = attrib.texcoords[2*i2.texcoord_index+1];
    }
    
    // bind vertex array object
    glBindVertexArray(_vertexArrayObject);
    
    // generate vertices buffer
    GLuint verticesBuffer;
    glGenBuffers(1, &verticesBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, verticesBuffer);
    glBufferData(GL_ARRAY_BUFFER, 3*_numberOfVertices*sizeof(float),
                 _vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(0);
    
    // generate normals buffer
    GLuint normalsBuffer;
    glGenBuffers(1, &normalsBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalsBuffer);
    glBufferData(GL_ARRAY_BUFFER, 3*_numberOfVertices*sizeof(float),
                 normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(1);
    
    // generate texture coordinates buffer
    GLuint textureCoordinatesBuffer;
    glGenBuffers(1, &textureCoordinatesBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, textureCoordinatesBuffer);
    glBufferData(GL_ARRAY_BUFFER, 2*_numberOfVertices*sizeof(float),
                 textureCoordinates.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(2);
    
    return true;
  }
  return false;
}

bool GraphicsComponent::loadTexture(const char * textureFileName,
                                    TextureType textureType)
{
  if (_shaderProgram)
  {
    // load image
    int width, height, components;
    auto image = stbi_load(textureFileName, &width, &height, &components,
                           STBI_rgb_alpha);
    if (image) {
      // determine texture type
      GLuint * texture = nullptr;
      if (textureType == Diffuse)
        texture = &_diffuseMap;
      else
        // TODO: implement more texture types
        return nullptr;
      
      // generate texture
      glGenTextures(1, texture);
      glBindTexture(GL_TEXTURE_2D, *texture);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                   GL_UNSIGNED_BYTE, image);
      free(image);
      
      // set filtering options and attach texture to shader
      glUseProgram(_shaderProgram);
      if (textureType == Diffuse)
      {
        glGenerateMipmap(GL_TEXTURE_2D);
        glUniform1i(glGetUniformLocation(_shaderProgram, "diffuseMap"), 0);
      }
      else
      {
        // other texture types
      }
      
      return true;
    }
  }
  return false;
}

bool GraphicsComponent::loadShader(const char * vertexShaderFileName,
                                   const char * fragmentShaderFileName)
{
  auto result = Core::CreateShaderProgram(vertexShaderFileName,
                                          fragmentShaderFileName);
  if (!result.isNothing())
  {
    _shaderProgram = result;
    
    // retrieve uniform locations
    _modelMatrixLocation =
      glGetUniformLocation(_shaderProgram, "modelMatrix");
    _modelViewProjectionMatrixLocation =
      glGetUniformLocation(_shaderProgram, "modelViewProjectionMatrix");
    _normalMatrixLocation =
      glGetUniformLocation(_shaderProgram, "normalMatrix");
    _diffuseColorLocation =
      glGetUniformLocation(_shaderProgram, "diffuseColor");
    
    return true;
  }
  return false;
}

string GraphicsComponent::trait() const { return "Graphics"; }

void GraphicsComponent::diffuseColor(const vec3 & color)
{
  _diffuseColor = color;
}


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
  
  // set flags
  glEnable(GL_PROGRAM_POINT_SIZE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
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
  const int amount = std::min(_maxNumberOfParticles-(int)_particles.size(), 64);
  for (int i = 0; i < amount; ++i)
  {
    const float theta = Core::uniformRandom(0.0f, 2.0f*M_PI);
    const float u     = Core::uniformRandom(0.95f, 1.0f);
    const float c     = sqrt(1.0f - u*u);
    const vec4 p = model * vec4(0.0f, 0.0f, 0.0f, 1.0f);
    const vec3 v = vec3(model * vec4(u, c*cosf(theta), c*sinf(theta), 0.0f));
    _particles.push_back({0.0f, 1.0f, vec3(p)/p.w, 2.0f*v});
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
    float dt           = core.deltaTime();
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
  glUniformMatrix4fv(_projectionMatrixLocation, 1, false, &projection[0].x);
  glUniform1f(_screenWidthLocation, d.x);
  glUniform1f(_screenHeightLocation, d.y);
  
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
      glUniform1i(glGetUniformLocation(_shaderProgram, "diffuseMap"), 0);
      
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
    _projectionMatrixLocation =
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


// MARK: -
// MARK: Properties
const Core * Entity::core() const                { return _core; }
const Entity * Entity::parent() const            { return _parent; }
const vector<Entity*> & Entity::children() const { return _children; }

InputComponent * Entity::input() const         { return _input; }
AnimationComponent * Entity::animation() const { return _animation; }
ColliderComponent * Entity::collider() const   { return _collider; }
RigidBodyComponent * Entity::rigidBody() const { return _rigidBody; }
AudioComponent * Entity::audio() const         { return _audio; }
GraphicsComponent * Entity::graphics() const   { return _graphics; }
ParticleSystemComponent * Entity::particleSystem() const
{
  return _particleSystem;
}

const vec3 & Entity::localPosition() const    { return _localPosition; }
const quat & Entity::localOrientation() const { return _localOrientation; }
const vec3 & Entity::localScale() const       { return _localScale; }
const vec3 & Entity::velocity() const         { return _velocity; }
const vec3 & Entity::force() const            { return _force; }

bool Entity::enabled() const            { return _enabled; }
const EntityType & Entity::type() const { return _type; }

// MARK: Member functions
Entity::Entity()
  : _core(nullptr)
  , _parent(nullptr)
  , _children()
  , _input(nullptr)
  , _animation(nullptr)
  , _collider(nullptr)
  , _rigidBody(nullptr)
  , _audio(nullptr)
  , _graphics(nullptr)
  , _particleSystem(nullptr)
  , _localPosition(0.0f)
  , _localOrientation(1.0f, 0.0f, 0.0f, 0.0f)
  , _localScale(1.0f, 1.0f, 1.0f)
  , _velocity(0.0f)
  , _force(0.0f)
  , _enabled(false)
  , _type(Default)
  , _transformNeedsUpdating(true)
{}

void Entity::init(Core * core)
{
  _core = core;
  _enabled = true;
  
  if (_input)          _input->init(this);
  if (_animation)      _animation->init(this);
  if (_collider)       _collider->init(this);
  if (_rigidBody)      _rigidBody->init(this);
  if (_audio)          _audio->init(this);
  if (_graphics)       _graphics->init(this);
  if (_particleSystem) _particleSystem->init(this);
  
  if (_parent)
  {
    auto eventHandler = [this](Event)
    {
      _transformNeedsUpdating = true;
      NotificationCenter::notify(DidUpdateTransform, *this);
    };
    NotificationCenter::observe(eventHandler, DidUpdateTransform, _parent);
  }
  
  for (auto child : _children) child->init(core);
}

void Entity::reset()
{
  if (_input)          _input->reset();
  if (_animation)      _animation->reset();
  if (_collider)       _collider->reset();
  if (_rigidBody)      _rigidBody->reset();
  if (_audio)          _audio->reset();
  if (_graphics)       _graphics->reset();
  if (_particleSystem) _particleSystem->reset();
  
  for (auto child : _children) child->reset();
}

void Entity::destroy()
{
  for (auto child : _children)
  {
    child->destroy();
  }
  _children.clear();
  
  if (_input)          delete _input;
  if (_animation)      delete _animation;
  if (_collider)       delete _collider;
  if (_rigidBody)      delete _rigidBody;
  if (_audio)          delete _audio;
  if (_graphics)       delete _graphics;
  if (_particleSystem) delete _particleSystem;
}

void Entity::addChild(Entity * child)
{
  _children.push_back(child);
  child->_parent = this;
}

Entity * Entity::findChild(string id)
{
  for (auto child : _children)
  {
    if (child->id().compare(id) == 0)
      return child;
    auto possible_find = child->findChild(id);
    if (possible_find)
      return possible_find;
  }
  return nullptr;
}

void Entity::removeChild(string id)
{
  for (int i = 0; i < _children.size(); i++)
  {
    auto child = _children[i];
    if (child->id() == id)
    {
      child->_parent = nullptr;
      _children.erase(_children.begin()+i);
    }
  }
}

void Entity::attachInputComponent(InputComponent * input)
{
  _input = input;
}

void Entity::attachAnimationComponent(AnimationComponent * animation)
{
  _animation = animation;
}

void Entity::attachRigidBodyComponent(RigidBodyComponent * rigidBody)
{
  _rigidBody = rigidBody;
}

void Entity::attachColliderComponent(ColliderComponent * collider)
{
  _collider = collider;
}

void Entity::attachAudioComponent(AudioComponent * audio)
{
  _audio = audio;
}

void Entity::attachGraphicsComponent(GraphicsComponent * graphics)
{
  _graphics = graphics;
}

void Entity::attachParticleSystemComponent
  (ParticleSystemComponent * particleSystem)
{
  _particleSystem = particleSystem;
}

mat4 Entity::localTransform() const
{
  return localTranslation() * localRotation();
}

mat4 Entity::localTranslation() const
{
  return glm::translate(_localPosition);
}

mat4 Entity::localRotation() const
{
  return glm::toMat4(_localOrientation);
}

vec3 Entity::localUp() const
{
  return glm::rotate(_localOrientation, Core::WORLD_UP);
}

vec3 Entity::localDown() const
{
  return glm::rotate(_localOrientation, Core::WORLD_DOWN);
}

vec3 Entity::localLeft() const
{
  return glm::rotate(_localOrientation, Core::WORLD_LEFT);
}

vec3 Entity::localRight() const
{
  return glm::rotate(_localOrientation, Core::WORLD_RIGHT);
}

vec3 Entity::localForward() const
{
  return glm::rotate(_localOrientation, Core::WORLD_FORWARD);
}

vec3 Entity::localBackward() const
{
  return glm::rotate(_localOrientation, Core::WORLD_BACKWARD);
}

const vec3 & Entity::worldPosition() const
{
  if (_transformNeedsUpdating) _updateTransform();
  return _worldPosition;
}

const quat & Entity::worldOrientation() const
{
  if (_transformNeedsUpdating) _updateTransform();
  return _worldOrientation;
}

const vec3 & Entity::worldScale() const
{
  if (_transformNeedsUpdating) _updateTransform();
  return _worldScale;
}

mat4 Entity::worldTransform() const
{
  return worldTranslation() * worldRotation() * worldScaling();
}

mat4 Entity::worldTranslation() const
{
  return glm::translate(worldPosition());
}

mat4 Entity::worldRotation() const
{
  return glm::toMat4(worldOrientation());
}

mat4 Entity::worldScaling() const
{
  return glm::scale(worldScale());
}

void Entity::translate(const vec3 & d)
{
  _localPosition += d;
  _transformNeedsUpdating = true;
  NotificationCenter::notify(DidUpdateTransform, *this);
}

void Entity::rotate(float angle, const vec3 & axis)
{
  quat newQuat = glm::angleAxis(angle, axis) * _localOrientation;
  _localOrientation = glm::normalize(newQuat);
  _transformNeedsUpdating = true;
  NotificationCenter::notify(DidUpdateTransform, *this);
}

void Entity::scale(float s)
{
  scale({s, s, s});
}

void Entity::scale(const vec3 & s)
{
  _localScale *= s;
  _transformNeedsUpdating = true;
  NotificationCenter::notify(DidUpdateTransform, *this);
}

void Entity::accelerate(const vec3 & v)
{
  _velocity += v;
}

void Entity::applyForce(const vec3 & f)
{
  _force += f;
}

void Entity::reposition(const vec3 & p)
{
  _localPosition.x = 0.0f;
  _localPosition.y = 0.0f;
  _localPosition.z = 0.0f;
  translate(p);
}

void Entity::repositionX(float x)
{
  _localPosition.x = 0.0f;
  translate({x, 0.0f, 0.0f});
}

void Entity::repositionY(float y)
{
  _localPosition.y = 0.0f;
  translate({0.0f, y, 0.0f});
}

void Entity::repositionZ(float z)
{
  _localPosition.z = 0.0f;
  translate({0.0f, 0.0f, z});
}

void Entity::reorient(const vec3 & o)
{
  quat qX = glm::angleAxis(o.x, Core::WORLD_RIGHT);
  quat qY = glm::angleAxis(o.y, Core::WORLD_UP);
  quat qZ = glm::angleAxis(o.z, Core::WORLD_BACKWARD);
  _localOrientation = qZ * qY * qX;
  _transformNeedsUpdating = true;
  NotificationCenter::notify(DidUpdateTransform, *this);
}

void Entity::resetPitch(float pitch)
{
  vec3 eulerAngles = glm::eulerAngles(_localOrientation);
  reorient({pitch, eulerAngles.y, eulerAngles.z});
}

void Entity::resetYaw(float yaw)
{
  vec3 eulerAngles = glm::eulerAngles(_localOrientation);
  reorient({eulerAngles.x, yaw, eulerAngles.z});
}

void Entity::resetRoll(float roll)
{
  vec3 eulerAngles = glm::eulerAngles(_localOrientation);
  reorient({eulerAngles.x, eulerAngles.y, roll});
}

void Entity::rescale(const vec3 & s)
{
  _localScale.x = 1.0f;
  _localScale.y = 1.0f;
  _localScale.z = 1.0f;
  scale(s);
}

void Entity::rescaleX(float x)
{
  _localScale.x = 1.0f;
  scale({x, 1.0f, 1.0f});
}

void Entity::rescaleY(float y)
{
  _localScale.y = 1.0f;
  translate({1.0f, y, 1.0f});
}

void Entity::rescaleZ(float z)
{
  _localScale.z = 1.0f;
  translate({1.0f, 1.0f, z});
}

void Entity::resetVelocity(const vec3 & v)
{
  _velocity.x = 0.0f;
  _velocity.y = 0.0f;
  _velocity.z = 0.0f;
  accelerate(v);
}

void Entity::resetVelocityX(float vx)
{
  _velocity.x = 0.0f;
  accelerate({vx, 0.0f, 0.0f});
}

void Entity::resetVelocityY(float vy)
{
  _velocity.y = 0.0f;
  accelerate({0.0f, vy, 0.0f});
}

void Entity::resetVelocityZ(float vz)
{
  _velocity.z = 0.0f;
  accelerate({0.0f, 0.0f, vz});
}

void Entity::resetForce(const vec3 & f)
{
  _force.x = 0.0f;
  _force.y = 0.0f;
  _force.z = 0.0f;
  applyForce(f);
}

void Entity::resetForceX(float fx)
{
  _force.x = 0.0f;
  applyForce({fx, 0.0f, 0.0f});
}

void Entity::resetForceY(float fy)
{
  _force.y = 0.0f;
  applyForce({0.0f, fy, 0.0f});
}

void Entity::resetForceZ(float fz)
{
  _force.z = 0.0f;
  applyForce({0.0f, 0.0f, fz});
}

void Entity::type(const EntityType & newType)
{
  _type = newType;
}

bool Entity::operator ==(Entity & entity)
{
  return id().compare(entity.id()) == 0;
}

bool Entity::operator !=(Entity & entity)
{
  return id().compare(entity.id()) != 0;
}

// MARK: Private
void Entity::_updateTransform() const
{
  if (_parent)
  {
    _worldPosition     = _parent->worldPosition() + _localPosition;
    _worldOrientation  = _parent->worldOrientation() * _localOrientation;
    _worldScale        = _parent->worldScale() * _localScale;
  }
  else
  {
    _worldPosition    = _localPosition;
    _worldOrientation = _localOrientation;
    _worldScale       = _localScale;
  }
  _transformNeedsUpdating = false;
}


// MARK: -
// MARK: Member functions
PointLight::PointLight(float distance)
  : Entity()
  , _linear(4.55f/distance)
  , _quadratic(75.0f/(distance*distance))
{}


// MARK: -
// MARK: Properties
const Entity & Core::root() const { return _root; };
Entity * Core::camera() const     { return _camera; };

double Core::deltaTime() const            { return _deltaTime; };
const ivec2 & Core::mousePosition() const { return _mousePosition; };
const ivec2 & Core::mouseMovement() const { return _mouseMovement; };

int Core::sampleRate() const   { return _sampleRate; };
double Core::maxVolume() const { return _maxVolume; };

const mat4 & Core::viewMatrix() const       { return _viewMatrix; };
const mat4 & Core::projectionMatrix() const { return _projectionMatrix; };

int Core::scale() const                    { return _scale; };
const vec3 & Core::backgroundColor() const { return _backgroundColor; };

const vec3 Core::WORLD_UP       { 0.0f,  1.0f,  0.0f};
const vec3 Core::WORLD_DOWN     { 0.0f, -1.0f,  0.0f};
const vec3 Core::WORLD_LEFT     {-1.0f,  0.0f,  0.0f};
const vec3 Core::WORLD_RIGHT    { 1.0f,  0.0f,  0.0f};
const vec3 Core::WORLD_FORWARD  { 0.0f,  0.0f, -1.0f};
const vec3 Core::WORLD_BACKWARD { 0.0f,  0.0f,  1.0f};

// MARK: Class functions
float Core::uniformRandom(float from, float to)
{
  return from+(to-from)*rand()/RAND_MAX;
}

bool Core::AABBIntersect(const box & a, const box & b, box & intersection)
{
  for (int i = 0; i < 3; i++)
  {
    const float aMin_i = a.min[i];
    const float aMax_i = a.max[i];
    const float bMin_i = b.min[i];
    const float bMax_i = b.max[i];
    
    if (aMin_i > bMax_i || bMin_i > aMax_i)
      return false;
    
    intersection.min[i] = aMin_i > bMin_i ? aMin_i : bMin_i;
    intersection.max[i] = aMax_i < bMax_i ? aMax_i : bMax_i;
  }
  
  return true;
}


bool Core::SphereIntersect(const vec3 & p1,
                           const vec3 & p2,
                           float r1,
                           float r2,
                           vec3 & intersection)
{
  vec3 d = p2-p1;
  const float diff = r1 + r2 - glm::length(d);
  if (diff < 0)
    return false;
  intersection = diff * glm::normalize(d);
  return true;
}

bool Core::SphereCollision(const vec3 & o1,
                           const vec3 & o2,
                           const vec3 & v1,
                           const vec3 & v2,
                           float r1,
                           float r2,
                           float & time,
                           vec3 & p1,
                           vec3 & p2)
{
  const vec3 v  = v1-v2;
  const vec3 l    = o1-o2;
  const float r = r1+r2;
  const float a = glm::dot(v, v);
  if (a == 0.0f)
    return false;
  const float b   = 2.0f*glm::dot(l, v);
  const float c   = glm::dot(l, l) - r*r;
  const float d   = b*b - 4.0f*a*c;
  if (d < 0.0f)
    return false;
  const float q   = -0.5f*(b + glm::sign(b)*glm::sqrt(d));
  if (q == 0.0f)
    return false;
  const float t0  = q/a;
  const float t1  = c/q;
  time = glm::min(t0, t1);
  p1   = o1 + time*v1;
  p2   = o2 + time*v2;
  return true;
}

string _shaderInfoLog(GLuint object)
{
  int logLength = 0;
  string log = "UNKNOWN ERROR";
  glGetShaderiv(object, GL_INFO_LOG_LENGTH, &logLength);
  if (logLength > 0)
  {
    char * tmpLog = new char[logLength];
    glGetShaderInfoLog(object, logLength, &logLength, tmpLog);
    log = tmpLog;
    delete[] tmpLog;
  }
  return log;
}

bool _compileShader(GLuint shader, string shaderFileName)
{
  int success;
  cout << "Compiling '" << shaderFileName << "'... ";
  glCompileShader(shader);
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    cout << endl << "└─> " << _shaderInfoLog(shader) << endl;
    return false;
  }
  cout << "ok" << endl;
  return true;
}

bool _linkProgram(GLuint program)
{
  int success;
  cout << "Linking... ";
  glLinkProgram(program);
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success)
  {
    cout << endl << "└─> " << _shaderInfoLog(program) << endl;
    return false;
  }
  cout << "ok" << endl;
  return true;
}

maybe<GLuint> Core::CreateShaderProgram(const char * vertexShaderFileName,
                                        const char * fragmentShaderFileName)
{
  GLuint vertexShader   = glCreateShader(GL_VERTEX_SHADER);
  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  
  ifstream vertexShaderFile(vertexShaderFileName);
  string vertexShaderSource((istreambuf_iterator<char>(vertexShaderFile)),
                            istreambuf_iterator<char>());
  
  ifstream fragmentShaderFile(fragmentShaderFileName);
  string fragmentShaderSource((istreambuf_iterator<char>(fragmentShaderFile)),
                              istreambuf_iterator<char>());
  
  const char * vertexShaderSourceStr   = vertexShaderSource.c_str();
  const char * fragmentShaderSourceStr = fragmentShaderSource.c_str();
  
  glShaderSource(vertexShader, 1, &vertexShaderSourceStr, nullptr);
  glShaderSource(fragmentShader, 1, &fragmentShaderSourceStr, nullptr);
  
  if (!_compileShader(vertexShader,   vertexShaderFileName) ||
      !_compileShader(fragmentShader, fragmentShaderFileName))
    return maybe<GLuint>::nothing();
  
  GLuint shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, fragmentShader);
  glAttachShader(shaderProgram, vertexShader);
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
  CHECK_GL_ERROR(true);
  if (!_linkProgram(shaderProgram))
    return maybe<GLuint>::nothing();
  
  return maybe<GLuint>::just(shaderProgram);
}

bool Core::CheckGLError(bool fatal)
{
  bool error = false;
  for (GLenum err = glGetError(); err != GL_NO_ERROR; err = glGetError())
  {
    error = fatal;
    cout << "OpenGL " << (fatal ? "error: " : "warning: ");
    cout << gluErrorString(err) << endl;
  }
  return error;
}

// MARK: Member functions
Core::Core(int numberOfEntities)
  : _mousePosition(0, 0)
  , _mouseMovement(0, 0)
  , _sampleRate(44100)
  , _maxVolume(0.05)
  , _scale(1)
  , _backgroundColor(0.0f, 0.0f, 0.0f)
  , _entityCount(0)
  , _maximumNumberOfEntities(numberOfEntities)
  , _reset(false)
  , _pause(false)
{
  _entities.resize(_maximumNumberOfEntities);
  _root.assignIdentifier("root");
  _camera = createEntity("camera");
}

bool Core::init(CoreOptions & options)
{
  // TODO: make more secure, i.e. handle errors better
  
  // initialize SDL
  if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
  {
    SDL_Log("SDL_Init: %s\n", SDL_GetError());
    return false;
  }
  SDL_GL_LoadLibrary(nullptr);
  
  // initialize OpenGL
  SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
#ifdef __APPLE__
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                      SDL_GL_CONTEXT_PROFILE_CORE);
#elif defined _WIN32
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                      SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
#endif
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  
  // create window
  auto windowOptions = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI;
  _window = SDL_CreateWindow(options.title,
                             SDL_WINDOWPOS_UNDEFINED,
                             SDL_WINDOWPOS_UNDEFINED,
                             options.width,
                             options.height,
                             windowOptions);
  if (_window == nullptr)
  {
    SDL_Log("SDL_CreateWindow: %s\n", SDL_GetError());
    return false;
  }
  SDL_SetRelativeMouseMode(SDL_TRUE);
  
  // create context for window
  _context = SDL_GL_CreateContext(_window);
  if (_context == nullptr) {
    SDL_Log("SDL_GL_CreateContext: %s\n", SDL_GetError());
    return false;
  }
  
  // initialize glew
#ifdef __APPLE__
  glewExperimental = true;
#endif
  glewInit();
  CHECK_GL_ERROR(false);
  
  // enable v-sync
  SDL_GL_SetSwapInterval(1);
  
  // flip textures
  stbi_set_flip_vertically_on_load(true);
  
  // clear and swap frame
  glClear(GL_COLOR_BUFFER_BIT);
  SDL_GL_SwapWindow(_window);
  
  // generate vertex array object for fullscreen quad
  glGenVertexArrays(1, &_quadVertexArrayObject);
  glBindVertexArray(_quadVertexArrayObject);
  
  vector<float> quadPositions =
  {
    -1.0f, -1.0f,
     1.0f, -1.0f,
    -1.0f,  1.0f,
     1.0f,  1.0f
  };
  GLuint quadPositionsBuffer;
  glGenBuffers(1, &quadPositionsBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, quadPositionsBuffer);
  glBufferData(GL_ARRAY_BUFFER, quadPositions.size()*sizeof(float),
               quadPositions.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, false, 0, 0);
  glEnableVertexAttribArray(0);
  
  // generate geometry buffer
  glGenFramebuffers(1, &_geometryBuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, _geometryBuffer);
  
  // generate textures for geometry buffer
  const ivec2 d = viewDimensions();
  glGenTextures(1, &_geometryPositionMap);
  glBindTexture(GL_TEXTURE_2D, _geometryPositionMap);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, d.x, d.y, 0,
               GL_RGB, GL_FLOAT, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         _geometryPositionMap, 0);
  
  glGenTextures(1, &_geometryNormalMap);
  glBindTexture(GL_TEXTURE_2D, _geometryNormalMap);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, d.x, d.y, 0,
               GL_RGB, GL_FLOAT, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
                         _geometryNormalMap, 0);
  
  glGenTextures(1, &_geometryColorMap);
  glBindTexture(GL_TEXTURE_2D, _geometryColorMap);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, d.x, d.y, 0,
               GL_RGB, GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D,
                         _geometryColorMap, 0);
  
  GLuint attachments[3] =
  {
    GL_COLOR_ATTACHMENT0,
    GL_COLOR_ATTACHMENT1,
    GL_COLOR_ATTACHMENT2
  };
  glDrawBuffers(3, attachments);
  
  // generate render buffer
  GLuint renderBuffer;
  glGenRenderbuffers(1, &renderBuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, d.x, d.y);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, renderBuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
  // create light shader program
  auto result = CreateShaderProgram("shaders/deferred_lighting.vert",
                                    "shaders/deferred_lighting.frag");
  if (result.isNothing()) return false;
  _lightingPass = result;
  
  // set shader samplers
  glUseProgram(_lightingPass);
  glUniform1i(glGetUniformLocation(_lightingPass, "gPosition"), 0);
  glUniform1i(glGetUniformLocation(_lightingPass, "gNormal"), 1);
  glUniform1i(glGetUniformLocation(_lightingPass, "gColor"), 2);
  
  // initialize entities
  _root.init(this);
  _root.reset();
  
  // store collider entity references
  for (auto & entity : _entities)
    if (entity.collider()) _colliders.push_back(&entity);
  
  // initialize data structures for the sweep and prune algorithm
  const long size = _colliders.size();
  _intervalBounds[0].resize(2*size);
  _intervalBounds[1].resize(2*size);
  _intervalBounds[2].resize(2*size);
  _intervalPairOverlaps[0].resize(size-1);
  _intervalPairOverlaps[1].resize(size-1);
  _intervalPairOverlaps[2].resize(size-1);
  for (int i = 0; i < size; ++i)
  {
    auto & aabb = _colliders[i]->collider()->dynamicAxisAlignedBoundingBox();
    _intervalBounds[0][2*i]   = {_IntervalBound::START, i, &aabb.min.x};
    _intervalBounds[0][2*i+1] = {_IntervalBound::END,   i, &aabb.max.x};
    _intervalBounds[1][2*i]   = {_IntervalBound::START, i, &aabb.min.y};
    _intervalBounds[1][2*i+1] = {_IntervalBound::END,   i, &aabb.max.y};
    _intervalBounds[2][2*i]   = {_IntervalBound::START, i, &aabb.min.z};
    _intervalBounds[2][2*i+1] = {_IntervalBound::END,   i, &aabb.max.z};
    if (i < size-1)
    {
      _intervalPairOverlaps[0][i].resize(size-1-i);
      _intervalPairOverlaps[1][i].resize(size-1-i);
      _intervalPairOverlaps[2][i].resize(size-1-i);
    }
  }
  
  // initialize audio
  auto fillStream = [](void * userdata, uint8_t * stream, int length)
  {
    Core * core         = (Core*)userdata;
    int16_t * stream16b = (int16_t*)stream;
    double maxVolume    = core->_maxVolume;
    
    for (int i = 0; i < length/2; i++) stream16b[i] = 0;
    
    function<void(Entity*)> callbacks;
    callbacks = [maxVolume, stream16b, length, &callbacks](Entity * entity)
    {
      // TODO: refactor audio
//      const AudioComponent * audio = entity->audio();
//      if (audio) audio->_audioStreamCallback(maxVolume, stream16b, length/2);
      
      for (auto child : entity->children()) callbacks(child);
    };
    
    callbacks(&core->_root);
  };
  
  SDL_AudioSpec desired_audio_spec;
  
  desired_audio_spec.freq     = _sampleRate;
  desired_audio_spec.format   = AUDIO_S16SYS;
  desired_audio_spec.channels = 1;
  desired_audio_spec.samples  = 2048;
  desired_audio_spec.callback = fillStream;
  desired_audio_spec.userdata = this;
  
//  SDL_OpenAudio(&desired_audio_spec, nullptr);
  
  SDL_PauseAudio(0);
  
  return true;
}

bool Core::update()
{
  bool shouldContinue = true;
  
  ////////////////////////////////////////
  // FRAME TIME SETUP
  ////////////////////////////////////////
  
  // record time
  static double prevTime = elapsedTime();
  double startTime = elapsedTime();
  _deltaTime = startTime - prevTime;
  prevTime = startTime;
  
  ////////////////////////////////////////
  // INPUT
  ////////////////////////////////////////
  
  // check user input
  _mouseMovement.x = 0;
  _mouseMovement.y = 0;
  SDL_Event event;
  set<SDL_Keycode> keysToPress, keysToRelease;
  while (SDL_PollEvent(&event))
  {
    switch (event.type)
    {
      case SDL_QUIT:
        shouldContinue = false;
        break;
      case SDL_KEYDOWN:
        keysToPress.insert(event.key.keysym.sym);
        break;
      case SDL_KEYUP:
        keysToRelease.insert(event.key.keysym.sym);
        break;
      case SDL_MOUSEMOTION:
        _mousePosition.x = event.motion.x;
        _mousePosition.y = event.motion.y;
        _mouseMovement.x = event.motion.xrel;
        _mouseMovement.y = event.motion.yrel;
        break;
    }
  }
  
  // update controls
  for (auto key : keysToPress)
  {
    for (auto & entry : _keyControls)
    {
      auto & control = entry.second;
      if (control.first == key) control.second = true;
    }
  }
  for (auto key : keysToRelease)
  {
    for (auto & entry : _keyControls)
    {
      auto & control = entry.second;
      if (control.first == key) control.second = false;
    }
  }
  
  // handle input for entities
  for (int i = 0; i < _entityCount; ++i)
  {
    auto & entity = _entities[i];
    if (entity.enabled() && entity.input())
      entity.input()->handleInput(*this);
  }
  
  ////////////////////////////////////////
  // ANIMATION
  ////////////////////////////////////////
  
  // animate entities
  for (int i = 0; i < _entityCount; ++i)
  {
    auto & entity = _entities[i];
    if (entity.enabled() && entity.animation())
      entity.animation()->animate(*this);
  }
  
  ////////////////////////////////////////
  // COLLISION DETECTION
  ////////////////////////////////////////
  
  // update colliders
  for (int i = 0; i < _entityCount; ++i)
  {
    auto & entity = _entities[i];
    if (entity.enabled() && entity.collider())
      entity.collider()->update(*this);
  }
  
  // sort collider bounding box intervals using insertion sort
  for (int d = 0; d < 3; ++d)
  {
    vector<_IntervalBound> & intervalBounds1D       = _intervalBounds[d];
    vector<vector<bool>> &   intervalPairOverlaps1D = _intervalPairOverlaps[d];
    for (int i = 1; i < intervalBounds1D.size(); ++i)
    {
      _IntervalBound * upper;
      _IntervalBound * lower = &intervalBounds1D[i];
      for (int j = i-1; j >= 0; --j)
      {
        upper = lower;
        lower = &intervalBounds1D[j];
        if (*upper->v < *lower->v)
        {
          std::swap(*upper, *lower);
          if (upper->type != lower->type)
          {
            int n = lower->i;
            int m = upper->i;
            if (n < m)
            {
              bool flipped = !intervalPairOverlaps1D[n][m-1];
              intervalPairOverlaps1D[n][m-1] = flipped;
            }
            else if (n > m)
            {
              bool flipped = !intervalPairOverlaps1D[m][n-1];
              intervalPairOverlaps1D[m][n-1] = flipped;
            }
          }
        }
        else break;
      }
    }
  }

  // collide entities
  for (int i = 0; i < _colliders.size(); ++i)
  {
    Entity * colliderEntity      = _colliders[i];
    ColliderComponent * collider = colliderEntity->collider();
    const vec3 colliderPosition  = colliderEntity->worldPosition();
    for (int j = i+1; j < _colliders.size(); ++j)
    {
      if (_intervalPairOverlaps[0][i][j-1] &&
          _intervalPairOverlaps[1][i][j-1] &&
          _intervalPairOverlaps[2][i][j-1])
      {
        Entity * obsticleEntity      = _colliders[j];
        ColliderComponent * obsticle = obsticleEntity->collider();
        const vec3 obsticlePosition  = obsticleEntity->worldPosition();
        const vec3 obsticleVelocity  = obsticleEntity->velocity();
        if (dynamic_cast<SphereColliderComponent*>(collider))
        {
          auto sphereCollider = (SphereColliderComponent*)collider;
          if (dynamic_cast<SphereColliderComponent*>(obsticle))
          {
            auto sphereObsticle = (SphereColliderComponent*)obsticle;
            vec3 intersection;
            if (SphereIntersect(colliderPosition,
                                obsticlePosition,
                                sphereCollider->radius(),
                                sphereObsticle->radius(),
                                intersection))
            {
              // TODO: better collision response
              obsticleEntity->resetVelocity(-obsticleVelocity);
              obsticleEntity->translate(intersection);
            }
          }
        }
      }
    }
  }
  
  ////////////////////////////////////////
  // PHYSICS
  ////////////////////////////////////////
  
  // update rigid bodies
  for (int i = 0; i < _entityCount; ++i)
  {
    auto & entity = _entities[i];
    if (entity.enabled() && entity.rigidBody())
      entity.rigidBody()->update(*this);
  }
  
  ////////////////////////////////////////
  // PRE-RENDERING
  ////////////////////////////////////////
  
  // retrieve view frame dimensions
  ivec2 d = viewDimensions();
  glViewport(0, 0, d.x, d.y);
  
  // enable depth test and face culling
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  
  // update view and projection matrix
  mat4 cameraTranslation    = glm::translate(-_camera->worldPosition());
  const mat4 cameraRotation = glm::inverse(_camera->worldRotation());
  _viewMatrix               = cameraRotation * cameraTranslation;
  _projectionMatrix         = glm::perspective(glm::radians(45.0f),
                                               float(d.x) / float(d.y),
                                               0.01f, 1000.0f);
  
  // clear default and geometry framebuffers
  glClearColor(_backgroundColor.r, _backgroundColor.g, _backgroundColor.b, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glBindFramebuffer(GL_FRAMEBUFFER, _geometryBuffer);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  // render entities to geometry buffer
  for (int i = 0; i < _entityCount; ++i)
  {
    auto & entity = _entities[i];
    if (entity.enabled() && entity.graphics() && entity.type() != Light)
      entity.graphics()->render(*this);
  }
  
  ////////////////////////////////////////
  // DEFERRED RENDERING
  ////////////////////////////////////////

  // bind textures
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glUseProgram(_lightingPass);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, _geometryPositionMap);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, _geometryNormalMap);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, _geometryColorMap);
  
  // update uniform data in shader
  for (int i = 0; i < _lights.size(); ++i)
  {
    Entity * light         = _lights[i];
    const vec3 position    = light->worldPosition();
    const vec3 color       = light->graphics()
      ? light->graphics()->diffuseColor()
      : vec3(1.0f);
    const float linear     = 0.22f;
    const float quadratic  = 0.20f;
    
    const string prefix    = "lights[" + to_string(i) + "].";
    const char * posName   = (prefix + "position").c_str();
    const char * colName   = (prefix + "color").c_str();
    const char * linName   = (prefix + "linear").c_str();
    const char * quadName  = (prefix + "quadratic").c_str();
    const GLuint posLoc    = glGetUniformLocation(_lightingPass, posName);
    const GLuint colLoc    = glGetUniformLocation(_lightingPass, colName);
    const GLuint linLoc    = glGetUniformLocation(_lightingPass, linName);
    const GLuint quadLoc   = glGetUniformLocation(_lightingPass, quadName);
    
    glUniform3fv(posLoc, 1, &position.x);
    glUniform3fv(colLoc, 1, &color.x);
    glUniform1f(linLoc, linear);
    glUniform1f(quadLoc, quadratic);
  }
  
  // draw fullscreen quad
  glBindVertexArray(_quadVertexArrayObject);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  
  // copy depth data to default framebuffer
  glBindFramebuffer(GL_READ_FRAMEBUFFER, _geometryBuffer);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glBlitFramebuffer(0, 0, d.x, d.y, 0, 0, d.x, d.y, GL_DEPTH_BUFFER_BIT,
                    GL_NEAREST);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // render light entities
  for (auto light : _lights)
  {
    if (light->enabled() && light->graphics())
      light->graphics()->render(*this);
  }

  // render particles
  for (int i = 0; i < _entityCount; ++i)
  {
    auto & entity = _entities[i];
    if (entity.enabled() && entity.particleSystem())
      entity.particleSystem()->render(*this);
  }
  
  // swap buffers
  SDL_GL_SwapWindow(_window);
  
  ////////////////////////////////////////
  // TIMERS
  ////////////////////////////////////////
  
  // possibly do a reset
  if (_reset)
  {
    _timers.clear();
    _root.reset();
    _reset = false;
    resume();
  }
  
  // go through timers
  int i = 0;
  while (i < _timers.size())
  {
    auto pair = _timers[i];
    const double currentTime = pair.second == _EFFECTIVE
      ? effectiveElapsedTime()
      : elapsedTime();
    if (currentTime >= pair.first.endTime)
    {
      pair.first.block();
      _timers.erase(_timers.begin() + i);
    }
    else i++;
  }
  
  return shouldContinue;
}

void Core::destroy()
{
  _root.destroy();
  
  SDL_CloseAudio();
  SDL_DestroyWindow(_window);
  SDL_Quit();
}

Entity * Core::createEntity(const string & id,
                            const string & parentId,
                            EntityType type)
{
  Entity * entity = nullptr;
  if (_entityCount < _maximumNumberOfEntities && !_root.findChild(id))
  {
    Entity * parent = &_root;
    if (parentId.compare("root") == 0 || (parent = _root.findChild(parentId)))
    {
      entity = &_entities[_entityCount++];
      if (type == Light) _lights.push_back(entity);
      entity->assignIdentifier(id);
      entity->type(type);
      parent->addChild(entity);
    }
  }
  return entity;
}

const Entity * Core::findEntity(const string & id) const
{
  for (const Entity & entity : _entities)
  {
    if (entity.id().compare(id) == 0)
      return &entity;
  }
  return nullptr;
}

void Core::createEffectiveTimer(double duration, function<void()> block)
{
  _timers.push_back
  ({
    {effectiveElapsedTime() + duration, block},
    _EFFECTIVE
  });
}

void Core::createAccumulativeTimer(double duration, function<void()> block)
{
  _timers.push_back
  ({
    {elapsedTime() + duration, block},
    _ACCUMULATIVE
  });
}

void Core::addControl(string name, SDL_Keycode key)
{
  _keyControls[name] = {key, false};
}

void Core::removeControl(string name)
{
  _keyControls.erase(name);
}

maybe<bool> Core::checkKey(string name) const
{
  if (_keyControls.find(name) != _keyControls.end())
    return maybe<bool>::just(_keyControls.at(name).second);
  return maybe<bool>::nothing();
  
}

void Core::reset(double after_duration)
{
  createAccumulativeTimer(after_duration, [this] { _reset = true; });
}

void Core::pause()
{
  _pause = true;
  effectiveElapsedTime();
}

void Core::resume()
{
  _pause = false;
  effectiveElapsedTime();
}

void Core::changeBackgroundColor(float r, float g, float b)
{
  _backgroundColor.r = r;
  _backgroundColor.g = g;
  _backgroundColor.b = b;
}

double Core::elapsedTime() const
{
  return SDL_GetTicks() / 1000.f;
}

double Core::effectiveElapsedTime() const
{
  static double lastPauseTime;
  static double totalPauseDuration;
  static bool pauseToggle;
  
  const double elapsed = elapsedTime();
  
  if (_pause && !pauseToggle)
  {
    pauseToggle = true;
    lastPauseTime = elapsed;
  }
  else if (!_pause && pauseToggle)
  {
    pauseToggle = false;
    totalPauseDuration += elapsed - lastPauseTime;
  }
  
  return (!_pause ? elapsed : lastPauseTime) - totalPauseDuration;
}

ivec2 Core::viewDimensions() const 
{
  ivec2 v;
  SDL_GL_GetDrawableSize(_window, &v.x, &v.y);
  return v;
}
