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
#include "imgui.h"
#include "imgui_impl_sdl_gl3.h"

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
  return _verts;
}
int GraphicsComponent::numberOfVertices() const
{
  return (int)_verts.size()/3;
}
bool GraphicsComponent::hasDiffuseTexture() const
{
  return _hasDiffTex;
}
const vec3 & GraphicsComponent::diffuseColor() const
{
  return _diffCol;
}
bool GraphicsComponent::deferredShading() const
{
  return _deferShading;
}

// MARK: Member functions
GraphicsComponent::GraphicsComponent()
  : Component()
  , _hasDiffTex(false)
  , _diffCol(0.8, 0.8, 0.8)
  , _deferShading(false)
{}

void GraphicsComponent::init(Entity * entity)
{
  Component::init(entity);
  
  glGenVertexArrays(1, &_vao);
}

void GraphicsComponent::render(const Core & core)
{
  // activate textures
  if (_hasDiffTex)
  {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _diffTexMap);
  }
  
  // render
  glBindVertexArray(_vao);
  glDrawArrays(GL_TRIANGLES, 0, (int)_verts.size());
}

bool GraphicsComponent::loadObject(const char * fileName)
{
  attrib_t           attrib;
  vector<shape_t>    shapes;
  string             error;
  LoadObj(&attrib, &shapes, nullptr, &error, fileName);

  if (shapes.size() >= 1 && !attrib.vertices.empty())
  {
    // initialize data collections
    vector<float> normals, texCoords;
    auto shape   = shapes[0];
    int numVerts = (int)shape.mesh.indices.size();
    _verts.resize(3*numVerts);
    normals.resize(3*numVerts);
    texCoords.resize(2*numVerts);
    
    // populate data collections
    for (int face = 0; face < numVerts/3; ++face)
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
      
      _verts[9*face]   = v0.x;
      _verts[9*face+1] = v0.y;
      _verts[9*face+2] = v0.z;
      _verts[9*face+3] = v1.x;
      _verts[9*face+4] = v1.y;
      _verts[9*face+5] = v1.z;
      _verts[9*face+6] = v2.x;
      _verts[9*face+7] = v2.y;
      _verts[9*face+8] = v2.z;
      
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
      texCoords[6*face]   = attrib.texcoords[2*i0.texcoord_index];
      texCoords[6*face+1] = attrib.texcoords[2*i0.texcoord_index+1];
      texCoords[6*face+2] = attrib.texcoords[2*i1.texcoord_index];
      texCoords[6*face+3] = attrib.texcoords[2*i1.texcoord_index+1];
      texCoords[6*face+4] = attrib.texcoords[2*i2.texcoord_index];
      texCoords[6*face+5] = attrib.texcoords[2*i2.texcoord_index+1];
    }
    
    // bind vertex array object
    glBindVertexArray(_vao);
    
    // generate vertices buffer
    GLuint vertsBuf;
    glGenBuffers(1, &vertsBuf);
    glBindBuffer(GL_ARRAY_BUFFER, vertsBuf);
    glBufferData(GL_ARRAY_BUFFER, 3*numVerts*sizeof(float), &_verts[0],
                 GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(0);
    
    // generate normals buffer
    GLuint normalsBuf;
    glGenBuffers(1, &normalsBuf);
    glBindBuffer(GL_ARRAY_BUFFER, normalsBuf);
    glBufferData(GL_ARRAY_BUFFER, 3*numVerts*sizeof(float), &normals[0],
                 GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(1);
    
    // generate texture coordinates buffer
    GLuint texCoordsBuf;
    glGenBuffers(1, &texCoordsBuf);
    glBindBuffer(GL_ARRAY_BUFFER, texCoordsBuf);
    glBufferData(GL_ARRAY_BUFFER, 2*numVerts*sizeof(float), &texCoords[0],
                 GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(2);
    
    return true;
  }
  return false;
}

bool GraphicsComponent::loadTexture(const char * fileName, TextureType texType)
{
  // load image
  int w, h, comps;
  auto image = stbi_load(fileName, &w, &h, &comps, STBI_rgb_alpha);
  if (image) {
    // determine texture type
    GLuint * texture = nullptr;
    if (texType == Diffuse)
    {
      texture     = &_diffTexMap;
      _hasDiffTex = true;
    }
    else
      // TODO: implement more texture types
      return nullptr;
    
    // generate texture
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, image);
    free(image);
    
    // set filtering options and attach texture to shader
    if (texType == Diffuse)
      glGenerateMipmap(GL_TEXTURE_2D);
    else
    {
      // other texture types
    }
    
    return true;
  }
  return false;
}

bool GraphicsComponent::loadShader(const char * vsfn, const char * fsfn)
{
  // TODO: implement
  return false;
}

string GraphicsComponent::trait() const { return "Graphics"; }

void GraphicsComponent::diffuseColor(const vec3 & col)
{
  _diffCol = col;
}

void GraphicsComponent::deferredShading(bool enabled)
{
  _deferShading = enabled;
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
    const float theta = Core::uniformRandom(0.0f, 2.0f*M_PI);
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
  glUniformMatrix4fv(_projMatrixLocation, 1, false, &projection[0].x);
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

const mat4 & Entity::previousWorldTransform() const
{
  return _previousWorldTransform;
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
  , _transformInvalid(true)
{}

void Entity::init(Core * core)
{
  // initialize properties
  _core    = core;
  _enabled = true;
  
  // initialize components
  if (_input)          _input->init(this);
  if (_animation)      _animation->init(this);
  if (_collider)       _collider->init(this);
  if (_rigidBody)      _rigidBody->init(this);
  if (_audio)          _audio->init(this);
  if (_graphics)       _graphics->init(this);
  if (_particleSystem) _particleSystem->init(this);
  
  // initial frame
  nextFrame();
  
  // set up notifications for when the transform of the entity changes
  if (_parent)
  {
    auto eventHandler = [this](Event)
    {
      _transformInvalid = true;
      NotificationCenter::notify(DidUpdateTransform, *this);
    };
    NotificationCenter::observe(eventHandler, DidUpdateTransform, _parent);
  }
  
  // recurse initialization on children
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

void Entity::nextFrame()
{
  _previousWorldTransform = worldTransform();
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
  if (_transformInvalid) _updateTransform();
  return _worldPosition;
}

const quat & Entity::worldOrientation() const
{
  if (_transformInvalid) _updateTransform();
  return _worldOrientation;
}

const vec3 & Entity::worldScale() const
{
  if (_transformInvalid) _updateTransform();
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
  _transformInvalid = true;
  NotificationCenter::notify(DidUpdateTransform, *this);
}

void Entity::rotate(float angle, const vec3 & axis)
{
  quat newQuat = glm::angleAxis(angle, axis) * _localOrientation;
  _localOrientation = glm::normalize(newQuat);
  _transformInvalid = true;
  NotificationCenter::notify(DidUpdateTransform, *this);
}

void Entity::scale(float s)
{
  scale({s, s, s});
}

void Entity::scale(const vec3 & s)
{
  _localScale *= s;
  _transformInvalid = true;
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
  _transformInvalid = true;
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
  _transformInvalid = false;
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
bool Core::mouseClick() const             { return _mouseClick; };
const ivec2 & Core::mousePosition() const { return _mousePosition; };
const ivec2 & Core::mouseMovement() const { return _mouseMovement; };

int Core::sampleRate() const   { return _sampleRate; };
double Core::maxVolume() const { return _maxVolume; };

const mat4 & Core::previousViewMatrix() const       { return _prevViewMatrix; };
const mat4 & Core::previousProjectionMatrix() const { return _prevProjMatrix; };
const mat4 & Core::viewMatrix() const               { return _viewMatrix; };
const mat4 & Core::projectionMatrix() const         { return _projMatrix; };

int Core::scale() const                    { return _scale; };
const vec3 & Core::backgroundColor() const { return _bgColor; };

int Core::particleSpawnRate() const  { return _particleSpawnRate; };
float Core::particleLifeTime() const { return _particleLifeTime; };
float Core::particleConeSize() const { return _particleConeSize; };
float Core::particleVelocity() const { return _particleVelocity; };

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
  CHECK_GL_ERROR(true);
  if (!success)
  {
    cout << endl << "└─> " << _shaderInfoLog(shader) << endl;
    return false;
  }
  cout << "ok" << endl;
  return true;
}

maybe<GLuint> Core::CreateShaderProgram(const char * vsfn, const char * fsfn)
{
  // shaders
  GLuint vs = glCreateShader(GL_VERTEX_SHADER);
  GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
  CHECK_GL_ERROR(true);
  
  // vertex shader source
  ifstream vsf(vsfn);
  string vss((istreambuf_iterator<char>(vsf)), istreambuf_iterator<char>());
  
  // fragment shader source
  ifstream fsf(fsfn);
  string fss((istreambuf_iterator<char>(fsf)), istreambuf_iterator<char>());
  
  // sources in c string format
  const char * vssCStr = vss.c_str();
  const char * fssCStr = fss.c_str();
  
  // bind sources to shaders
  glShaderSource(vs, 1, &vssCStr, nullptr);
  glShaderSource(fs, 1, &fssCStr, nullptr);
  CHECK_GL_ERROR(true);
  
  // compile and check if it succeeded
  if (!_compileShader(vs, vsfn) || !_compileShader(fs, fsfn))
    return maybe<GLuint>::nothing();
  
  // merge shaders into one program
  GLuint shaderProg = glCreateProgram();
  glAttachShader(shaderProg, fs);
  glAttachShader(shaderProg, vs);
  glDeleteShader(vs);
  glDeleteShader(fs);
  CHECK_GL_ERROR(true);
  
  // link program
  int success;
  cout << "Linking... ";
  glLinkProgram(shaderProg);
  glGetProgramiv(shaderProg, GL_LINK_STATUS, &success);
  CHECK_GL_ERROR(true);
  if (!success)
  {
    cout << endl << "└─> " << _shaderInfoLog(shaderProg) << endl;
    return maybe<GLuint>::nothing();
  }
  cout << "ok" << endl;
  
  return maybe<GLuint>::just(shaderProg);
}

bool Core::CheckGLError(bool fatal)
{
  bool error = false;
  for (GLenum err = glGetError(); err != GL_NO_ERROR; err = glGetError())
  {
    error = fatal;
    cout << "(OpenGL) " << (fatal ? "Error: " : "Warning: ") <<
      gluErrorString(err) << endl;
  }
  return error;
}

// MARK: Member functions
Core::Core(int numberOfEntities)
  : _mouseClick(false)
  , _mousePosition(0, 0)
  , _mouseMovement(0, 0)
  , _sampleRate(44100)
  , _maxVolume(0.05)
  , _scale(1)
  , _bgColor(0.0f, 0.0f, 0.0f)
  , _entityCount(0)
  , _maximumNumberOfEntities(numberOfEntities)
  , _reset(false)
  , _pause(false)
{
  _entities.resize(_maximumNumberOfEntities);
  _root.assignIdentifier("root");
  _camera = createEntity("camera");
}

bool Core::init(const CoreOptions & options)
{
  // TODO: make more secure, i.e. handle errors better
  
  // initialize frameworks
  if (!_initFrameworks(options.title, options.width, options.height))
    return false;
  
  // generate buffers
  if (!_generateBuffers())
    return false;
  
  // create shaders
  if (!_createDefaultShader() || !_createDeferredGeometryShader() ||
      !_createDeferredLightShader() || !_createDeferredAmbientShader() ||
      !_createPostMotionBlurShader() || !_createPostOutputShader())
    return false;
  
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
  
  // initialize GUI properties
  _motionBlurEnabled     = false;
  _motionBlurMode        = 0;
  _motionVelScaling      = 1.0f;
  _motionAdaptVarFPS     = true;
  _motionAdaptNumSamples = true;
  _motionPrefNumSamples  = 16;
  _deferredEnabled       = false;
  _deferShowQuads        = false;
  _deferAmbCol           = {0.3f, 0.3f, 0.3f};
  _deferNumLights        = (int)_lights.size();
  _deferAttDist          = 7.0f;
  _deferAttLin           = 0.12f;
  _deferAttQuad          = 0.58f;
  _particlesEnabled      = false;
  _particleSpawnRate     = 64;
  _particleLifeTime      = 1.0f;
  _particleConeSize      = 0.05f;
  _particleVelocity      = 5.0f;
  
  return true;
}

bool Core::update()
{
  
  ////////////////////////////////////////
  // SETUP
  ////////////////////////////////////////
  
  // record time
  static double prevTime = elapsedTime();
  double startTime = elapsedTime();
  _deltaTime = startTime - prevTime;
  prevTime = startTime;
  
  // create GUI frame
  ImGui_ImplSdlGL3_NewFrame(_window);
  
  ////////////////////////////////////////
  // INPUT
  ////////////////////////////////////////
  
  // check user input
  SDL_Event event;
  set<SDL_Keycode> keysToPress, keysToRelease;
  _mouseMovement.x = 0;
  _mouseMovement.y = 0;
  while (SDL_PollEvent(&event))
  {
    ImGui_ImplSdlGL3_ProcessEvent(&event);
    
    switch (event.type)
    {
      case SDL_MOUSEBUTTONDOWN:
        keysToPress.insert(event.button.button);
        break;
      case SDL_MOUSEBUTTONUP:
        keysToRelease.insert(event.button.button);
        break;
      case SDL_KEYDOWN:
        keysToPress.insert(event.key.keysym.sym);
        break;
      case SDL_KEYUP:
        keysToRelease.insert(event.key.keysym.sym);
        break;
      case SDL_QUIT:
        return false;
    }
    if (!ImGui::IsMouseHoveringWindow())
    {
      switch (event.type)
      {
        case SDL_MOUSEMOTION:
          _mousePosition.x = event.motion.x;
          _mousePosition.y = event.motion.y;
          _mouseMovement.x = event.motion.xrel;
          _mouseMovement.y = event.motion.yrel;
          break;
      }
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
  // RENDERING - SETUP
  ////////////////////////////////////////
  
  // retrieve view frame dimensions
  ivec2 d = viewDimensions();
  glViewport(0, 0, d.x, d.y);
  
  // update view and projection matrix
  _prevViewMatrix           = _viewMatrix;
  _prevProjMatrix           = _projMatrix;
  mat4 cameraTranslation    = glm::translate(-_camera->worldPosition());
  const mat4 cameraRotation = glm::inverse(_camera->worldRotation());
  _viewMatrix               = cameraRotation * cameraTranslation;
  _projMatrix               = glm::perspective(glm::radians(45.0f),
                                               float(d.x) / float(d.y),
                                               0.01f, 300.0f);
  const mat4 PV             = _projMatrix * _viewMatrix;
  const mat4 prevPV         = _prevProjMatrix * _prevViewMatrix;
  
  // define clear colors
  const GLfloat bgClear[]    = {_bgColor.r, _bgColor.g, _bgColor.b, 1.0f};
  const GLfloat blackClear[] = {0.0f,       0.0f,       0.0f,       1.0f};
  
  // clear frame buffers
  // FIXME: why is clearing so slow?
  CHECK_GL_ERROR(true);
  glClearBufferfv(GL_COLOR, 0, bgClear);
  glClear(GL_DEPTH_BUFFER_BIT);
  glBindFramebuffer(GL_FRAMEBUFFER, _postFBO);
  CHECK_GL_ERROR(true);
  if (!_clearPostProcessing(bgClear, blackClear))
    return false;
  
  ////////////////////////////////////////
  // RENDERING - (DEFERRED) GEOMETRY PASS
  ////////////////////////////////////////
  
  if (_deferredEnabled)
  {
    // disable blending and enable depth testing
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    
    // clear deferred shader frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, _deferFBO);
    glClearBufferfv(GL_COLOR, 0, blackClear);
    glClearBufferfv(GL_COLOR, 1, blackClear);
    glClearBufferfv(GL_COLOR, 2, bgClear);
    glClear(GL_DEPTH_BUFFER_BIT);
    CHECK_GL_ERROR(true);
    
    // render entities to geometry buffer
    for (int i = 0; i < _entityCount; ++i)
    {
      auto & entity = _entities[i];
      auto graphics = entity.graphics();
      if (entity.enabled() && entity.type() != Light && graphics &&
          graphics->deferredShading())
      {
        // construct transforms
        const mat4 M   = entity.worldTransform();
        const mat4 PVM = PV * M;
        const mat3 N   = glm::inverse(mat3(M));
        
        // set shader uniforms
        glUseProgram(_deferSh.prog);
        glUniformMatrix4fv(_deferSh.locs["M"],   1, false, &M[0].x);
        glUniformMatrix4fv(_deferSh.locs["PVM"], 1, false, &PVM[0].x);
        glUniformMatrix3fv(_deferSh.locs["N"],   1, true,  &N[0].x);
        CHECK_GL_ERROR(true);
        
        // render
        graphics->render(*this);
        CHECK_GL_ERROR(true);
      }
    }
  }
  
  ////////////////////////////////////////
  // RENDERING - (DEFERRED) AMBIENT PASS
  ////////////////////////////////////////
  
  if (_deferredEnabled)
  {
    // disable depth testing
    glDisable(GL_DEPTH_TEST);
    
    // bind shader program
    glBindFramebuffer(GL_FRAMEBUFFER, _postFBO);
    glUseProgram(_ambSh.prog);
    CHECK_GL_ERROR(true);
    
    // bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _deferColMap);
    CHECK_GL_ERROR(true);
    
    // update shader uniforms
    glUniform3fv(_ambSh.locs["ambCol"], 1, &_deferAmbCol.x);
    CHECK_GL_ERROR(true);
    
    // draw fullscreen quad
    glBindVertexArray(_quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    CHECK_GL_ERROR(true);
  }
  
  ////////////////////////////////////////
  // RENDERING - (DEFERRED) LIGHTING PASS
  ////////////////////////////////////////
  
  if (_deferredEnabled)
  {
    // enable additive blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    
    // enable depth testing, but disable writing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    
    // bind textures
    glUseProgram(_lightSh.prog);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _deferPosMap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, _deferNormMap);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, _deferColMap);
    CHECK_GL_ERROR(true);

    // update attenuation uniforms
    glUniform1i(_lightSh.locs["showQuad"], _deferShowQuads);
    glUniform1f(_lightSh.locs["attDist"],  _deferAttDist);
    glUniform1f(_lightSh.locs["attLin"],   _deferAttLin);
    glUniform1f(_lightSh.locs["attQuad"],  _deferAttQuad);
    CHECK_GL_ERROR(true);

    // render lighting
    const mat4 S        = glm::scale(vec3(_deferAttDist));
    const int numLights = std::min((int)_lights.size(), _deferNumLights);
    for (int i = 0; i < numLights; ++i)
    {
      
      // retrieve light data
      Entity * light = _lights[i];
      auto graphics  = light->graphics();
      const vec3 pos = light->worldPosition();
      const vec3 col = graphics ? graphics->diffuseColor() : vec3(1.0f);
     
      // compute quad transform
      const vec3 cameraDir = normalize(_camera->worldPosition()-pos);
      const mat4 T         = glm::translate(pos + _deferAttDist*cameraDir);
      const mat4 R         = glm::toMat4(_camera->worldOrientation());
      const mat4 PVM       = PV * T * R * S;
      
      // update shader uniforms
      glUniformMatrix4fv(_lightSh.locs["PVM"], 1, false, &PVM[0].x);
      glUniform3fv(_lightSh.locs["lightPos"], 1, &pos.x);
      glUniform3fv(_lightSh.locs["lightCol"], 1, &col.x);
      CHECK_GL_ERROR(true);

      // draw quad covering the light volume
      glBindVertexArray(_quadVAO);
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
      CHECK_GL_ERROR(true);
    }
  }
  
  ////////////////////////////////////////
  // RENDERING - ENTITIES
  ////////////////////////////////////////

  // disable blending and enable depth testing
  glDisable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
  
  // render entities using default shading
  for (int i = 0; i < _entityCount; ++i)
  {
    auto & entity = _entities[i];
    auto graphics = entity.graphics();
    if (entity.enabled() && graphics &&
        (!_deferredEnabled || !graphics->deferredShading() ||
         entity.type() == Light))
    {
      // construct transform
      const mat4 PVM     = PV * entity.worldTransform();
      const mat4 prevPVM = prevPV * entity.previousWorldTransform();
      
      // set shader uniforms
      int diffTexFlag = graphics->hasDiffuseTexture();
      glUseProgram(_defaultSh.prog);
      glUniformMatrix4fv(_defaultSh.locs["PVM"],     1, false, &PVM[0].x);
      glUniformMatrix4fv(_defaultSh.locs["prevPVM"], 1, false, &prevPVM[0].x);
      glUniform1i(_defaultSh.locs["hasDiffTexMap"], diffTexFlag);
      if (!diffTexFlag)
      {
        glUniform3fv(_defaultSh.locs["diffCol"], 1,
                     &graphics->diffuseColor().x);
      }
      CHECK_GL_ERROR(true);
      
      // render
      graphics->render(*this);
      CHECK_GL_ERROR(true);
    }
  }
  
  ////////////////////////////////////////
  // RENDERING - PARTICLES
  ////////////////////////////////////////
  // TODO: render particles after motion blur, with correct depth testing

  // enable transparecy blending
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  // render particles
  for (int i = 0; i < _entityCount; ++i)
  {
    auto & entity = _entities[i];
    if (_particlesEnabled && entity.enabled() && entity.particleSystem())
      entity.particleSystem()->render(*this);
    
    // last rendering pass using models, so advance to next frame
    entity.nextFrame();
  }
  if (!_swapPostProcessing())
    return false;
  
  ////////////////////////////////////////
  // POST-PROCESSING - MOTION BLUR
  ////////////////////////////////////////
  
  // disable blending and depth testing
  glDisable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  
  if (_motionBlurEnabled)
  {
    // bind shader program
    glUseProgram(_motionSh.prog);
    
    // bind color textore
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _postColMap[POST_IN]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, _postVelMap);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, _postDepthMap);
    CHECK_GL_ERROR(true);
    
    // construct transforms
    const mat4 currToPrev = prevPV * glm::inverse(PV);
    
    // set shader uniforms
    glUniformMatrix4fv(_motionSh.locs["currToPrev"], 1, false,
                       &currToPrev[0].x);
    glUniform1f(_motionSh.locs["fps"], 1.0f / _deltaTime);
    glUniform1i(_motionSh.locs["mode"], _motionBlurMode);
    glUniform1f(_motionSh.locs["velScaling"], _motionVelScaling);
    glUniform1i(_motionSh.locs["adaptVarFPS"], _motionAdaptVarFPS);
    glUniform1i(_motionSh.locs["adaptNumSamples"], _motionAdaptNumSamples);
    glUniform1i(_motionSh.locs["prefNumSamples"], _motionPrefNumSamples);
    CHECK_GL_ERROR(true);
    
    // draw fullscreen quad
    glBindVertexArray(_quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    CHECK_GL_ERROR(true);
    if (!_swapPostProcessing())
      return false;
  }
  
  ////////////////////////////////////////
  // POST-PROCESSING - OUTPUT
  ////////////////////////////////////////
  
  // bind shader program
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glUseProgram(_postOutputSh.prog);
  CHECK_GL_ERROR(true);
  
  // bind textures
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, _postColMap[POST_IN]);
  CHECK_GL_ERROR(true);
  
  // draw fullscreen quad
  glBindVertexArray(_quadVAO);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  CHECK_GL_ERROR(true);
  
  ////////////////////////////////////////
  // GUI
  ////////////////////////////////////////
  
  // calculate frame rate
  static double       frameRate      = 0.0;
  static int          frameCount     = 0;
  static double       timeStamp      = 0.0;
  static const double updateInterval = 0.25;
  const double        elapsed        = elapsedTime();
  ++frameCount;
  if (elapsed > timeStamp+updateInterval)
  {
    const double diff = elapsed-timeStamp;
    frameRate         = frameCount / diff;
    timeStamp        += int(diff/updateInterval)*updateInterval;
    frameCount        = 0;
  }
  
  // set controls
  ImGui::Text("%0.1f fps", frameRate);
  ImGui::Separator();
  ImGui::Checkbox("Motion blur", &_motionBlurEnabled);
  if (_motionBlurEnabled)
  {
    ImGui::Indent();
    ImGui::RadioButton("Camera", &_motionBlurMode, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Per object", &_motionBlurMode, 1);
    ImGui::Spacing();
    ImGui::SliderFloat("Velocity scaling", &_motionVelScaling, 0.0f, 10.0f);
    ImGui::Checkbox("Adapting to variable framerate", &_motionAdaptVarFPS);
    ImGui::Checkbox("Adaptive number of samples", &_motionAdaptNumSamples);
    ImGui::SliderInt("Max / preferred number of samples",
                     &_motionPrefNumSamples, 1, 32);
    ImGui::Unindent();
  }
  ImGui::Separator();
  ImGui::Checkbox("Deferred shading", &_deferredEnabled);
  if (_deferredEnabled)
  {
    ImGui::Indent();
    ImGui::Checkbox("Show quads", &_deferShowQuads);
    ImGui::ColorEdit3("Ambient color", &_deferAmbCol.r);
    ImGui::SliderInt("Number of lights", &_deferNumLights, 0,
                     (int)_lights.size());
    ImGui::Text("Light attenuation");
    ImGui::SliderFloat("Distance", &_deferAttDist, 0.0f, 10.0f);
    ImGui::SliderFloat("Linear term", &_deferAttLin, 0.0f, 5.0f);
    ImGui::SliderFloat("Quadratic term", &_deferAttQuad, 0.0f, 5.0f);
    ImGui::Unindent();
  }
  ImGui::Separator();
  ImGui::Checkbox("Particles", &_particlesEnabled);
  if (_particlesEnabled)
  {
    ImGui::Indent();
    ImGui::SliderInt("Spawn rate", &_particleSpawnRate, 0, 256);
    ImGui::SliderFloat("Life time", &_particleLifeTime, 0.0f, 10.0f);
    ImGui::SliderFloat("Cone size", &_particleConeSize, 0.01f, 1.0f);
    ImGui::SliderFloat("Velocity", &_particleVelocity, 0.1f, 10.0f);
    ImGui::Unindent();
  }
  ImGui::Separator();
  
  // render
  ImGui::Render();
  
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
  
  return true;
}

void Core::destroy()
{
  _root.destroy();
  
  ImGui_ImplSdlGL3_NewFrame(_window);
  ImGui_ImplSdlGL3_Shutdown();
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

void Core::addControl(string name, int key)
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
  _bgColor.r = r;
  _bgColor.g = g;
  _bgColor.b = b;
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

// MARK: Private
bool Core::_initFrameworks(const char * title, int scrnW, int scrnH)
{
  // initialize SDL
  if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
  {
    SDL_Log("SDL_Init: %s\n", SDL_GetError());
    return false;
  }
  SDL_GL_LoadLibrary(nullptr);
  
  // initialize OpenGL
  const int majorVer = 4;
#if defined(__APPLE__)
  const int minorVer = 1;
  const auto profile = SDL_GL_CONTEXT_PROFILE_CORE;
#elif defined(_WIN32)
  const int minorVer = 3;
  const auto profile = SDL_GL_CONTEXT_PROFILE_COMPATIBILITY;
#endif
  SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, majorVer);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minorVer);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, profile);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  
  // create window
  auto windowOptions = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI;
  _window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED,
                             SDL_WINDOWPOS_UNDEFINED, scrnW, scrnH,
                             windowOptions);
  if (_window == nullptr)
  {
    SDL_Log("SDL_CreateWindow: %s\n", SDL_GetError());
    return false;
  }
  //  SDL_SetRelativeMouseMode(SDL_TRUE);
  
  // create context for window
  _context = SDL_GL_CreateContext(_window);
  if (_context == nullptr) {
    cerr << "SDL error: " << SDL_GetError();
    return false;
  }
  
  // initialize glew
#ifdef __APPLE__
  glewExperimental = true;
#endif
  glewInit();
  CHECK_GL_ERROR(false);
  
  // initialize ImGUI
  if (!ImGui_ImplSdlGL3_Init(_window))
  {
    cerr << "ImGui error: failed to initialize.";
    return false;
  };
  
  // enable v-sync
  SDL_GL_SetSwapInterval(1);
  
  // flip textures
  stbi_set_flip_vertically_on_load(true);
  
  // set flags
  glEnable(GL_CULL_FACE);
  glEnable(GL_PROGRAM_POINT_SIZE);
  glBlendEquation(GL_FUNC_ADD);
  
  // clear and swap frame
  glClearColor(_bgColor.r, _bgColor.g, _bgColor.b, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  SDL_GL_SwapWindow(_window);
  
  return true;
}

bool _isFBOComplete(const string & name)
{
  int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status == GL_FRAMEBUFFER_COMPLETE)
    return true;

  string msg = "(OpenGL) Error: frame buffer object for " + name + " ";
  switch (status) {
    case GL_FRAMEBUFFER_UNDEFINED:
      msg += "is incomplete";
      break;
    case GL_FRAMEBUFFER_UNSUPPORTED:
      msg += "is unsupported.";
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
      msg += "has an attachment error.";
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
      msg += "has a missing attachment error.";
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
      msg += "has an incomplete draw buffer error.";
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
      msg += "has an incomplete read buffer error.";
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
      msg += "has a multisample error.";
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
      msg += "has a layer target error.";
      break;
    default:
      return false;
  }
  cerr << msg << endl;
  return false;
}

bool Core::_generateBuffers()
{
  const ivec2 d = viewDimensions();
  GLuint attachments[3] =
  {
    GL_COLOR_ATTACHMENT0,
    GL_COLOR_ATTACHMENT1,
    GL_COLOR_ATTACHMENT2
  };
  
  ////////////////////////////////////////
  // QUAD VERTEX ARRAY OBJECT
  ////////////////////////////////////////
  
  // generate vertex array object
  glGenVertexArrays(1, &_quadVAO);
  glBindVertexArray(_quadVAO);
  CHECK_GL_ERROR(true);
  
  // submit vertices to GPU
  vector<float> quadPos =
  {
    -1.0f, -1.0f,
    1.0f, -1.0f,
    -1.0f,  1.0f,
    1.0f,  1.0f
  };
  GLuint quadPosBuf;
  glGenBuffers(1, &quadPosBuf);
  glBindBuffer(GL_ARRAY_BUFFER, quadPosBuf);
  glBufferData(GL_ARRAY_BUFFER, quadPos.size()*sizeof(float),
               &quadPos[0], GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, false, 0, 0);
  glEnableVertexAttribArray(0);
  CHECK_GL_ERROR(true);
  
  ////////////////////////////////////////
  // POST PROCESSING
  ////////////////////////////////////////
  
  // generate color textures
  glGenTextures(2, _postColMap);
  for (auto i : {POST_IN, POST_OUT})
  {
    glBindTexture(GL_TEXTURE_2D, _postColMap[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, d.x, d.y, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    CHECK_GL_ERROR(true);
  }
  
  // generate velocity texture
  glGenTextures(1, &_postVelMap);
  glBindTexture(GL_TEXTURE_2D, _postVelMap);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, d.x, d.y, 0, GL_RG, GL_FLOAT,
               nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  CHECK_GL_ERROR(true);

  // generate depth texture
  glGenTextures(1, &_postDepthMap);
  glBindTexture(GL_TEXTURE_2D, _postDepthMap);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, d.x, d.y, 0,
               GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  CHECK_GL_ERROR(true);
  
  // generate frame buffer object
  glGenFramebuffers(1, &_postFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, _postFBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         _postColMap[POST_OUT], 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
                         _postVelMap, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                         _postDepthMap, 0);
  CHECK_GL_ERROR(true);
    
  // attach textures
  glDrawBuffers(2, attachments);
  CHECK_GL_ERROR(true);
    
  // check frame buffer status
  if (!_isFBOComplete("post-processing"))
  {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return false;
  }
  
  ////////////////////////////////////////
  // DEFERRED SHADING
  ////////////////////////////////////////
  
  // generate position texture
  glGenTextures(1, &_deferPosMap);
  glBindTexture(GL_TEXTURE_2D, _deferPosMap);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, d.x, d.y, 0, GL_RGB, GL_FLOAT,
               nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  CHECK_GL_ERROR(true);
  
  // generate normal texture
  glGenTextures(1, &_deferNormMap);
  glBindTexture(GL_TEXTURE_2D, _deferNormMap);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, d.x, d.y, 0, GL_RGB, GL_FLOAT,
               nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  CHECK_GL_ERROR(true);
  
  // generate color texture
  glGenTextures(1, &_deferColMap);
  glBindTexture(GL_TEXTURE_2D, _deferColMap);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, d.x, d.y, 0,
               GL_RGB, GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  CHECK_GL_ERROR(true);
  
  // generate frame buffer object
  glGenFramebuffers(1, &_deferFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, _deferFBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         _deferPosMap, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
                         _deferNormMap, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D,
                         _deferColMap, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                         _postDepthMap, 0);
  CHECK_GL_ERROR(true);
  
  // attach textures
  glDrawBuffers(3, attachments);
  CHECK_GL_ERROR(true);
  
  // check frame buffer status
  if (!_isFBOComplete("deferred shading"))
  {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return false;
  }
  
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  return true;
}

bool Core::_createShader(_Shader & sh, const char * vsfn, const char * fsfn,
                         const vector<string> & ids)
{
  auto result = CreateShaderProgram(vsfn, fsfn);
  
  if (!result.isNothing())
  {
    sh.prog = result;
    for (string id : ids)
      sh.locs[id] = glGetUniformLocation(sh.prog, id.c_str());
    CHECK_GL_ERROR(true);
    return true;
  }
  
  return false;
}

bool Core::_createDefaultShader()
{
  const vector<string> ids =
  {
    "PVM", "prevPVM", "diffTexMap", "hasDiffTexMap", "diffCol"
  };
  return _createShader(_defaultSh, "shaders/default.vert",
                       "shaders/default.frag", ids);
}

bool Core::_createDeferredGeometryShader()
{
  const vector<string> ids = {"M", "PVM", "N"};
  return _createShader(_deferSh, "shaders/deferred_geometry.vert",
                       "shaders/deferred_geometry.frag", ids);
}

bool Core::_createDeferredLightShader()
{
  const vector<string> ids =
  {
    "PVM", "posTexMap", "normTexMap", "diffTexMap", "showLightQuad", "lightPos",
    "lightCol", "attDist", "attLin", "attQuad"
  };
  bool success = _createShader(_lightSh, "shaders/deferred_lighting.vert",
                               "shaders/deferred_lighting.frag", ids);
  if (success)
  {
    // set texture uniforms
    glUseProgram(_lightSh.prog);
    glUniform1i(_lightSh.locs["posTexMap"],  0);
    glUniform1i(_lightSh.locs["normTexMap"], 1);
    glUniform1i(_lightSh.locs["diffTexMap"], 2);
    CHECK_GL_ERROR(true);
  }
  
  return success;
}

bool Core::_createDeferredAmbientShader()
{
  return _createShader(_ambSh, "shaders/quad.vert",
                       "shaders/deferred_ambient.frag",
                       {"diffTexMap", "ambCol"});
}

bool Core::_createPostMotionBlurShader()
{
  const vector<string> ids =
  {
    "diffTexMap", "velTexMap", "depthTexMap", "currToPrev", "fps", "mode",
    "velScaling", "adaptVarFPS", "adaptNumSamples", "prefNumSamples"
  };
  bool success = _createShader(_motionSh, "shaders/quad.vert",
                               "shaders/post_motion_blur.frag", ids);
  if (success)
  {
    // set texture uniforms
    glUseProgram(_motionSh.prog);
    glUniform1i(_motionSh.locs["diffTexMap"],  0);
    glUniform1i(_motionSh.locs["velTexMap"],   1);
    glUniform1i(_motionSh.locs["depthTexMap"], 2);
    CHECK_GL_ERROR(true);
  }
  
  return success;
}

bool Core::_createPostOutputShader()
{
  return _createShader(_postOutputSh, "shaders/quad.vert",
                       "shaders/post_output.frag", {"diffTexMap"});
}

inline bool Core::_swapPostProcessing()
{
  // swap color textures
  std::swap(_postColMap[POST_IN], _postColMap[POST_OUT]);
  
  // reattach texture
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         _postColMap[POST_OUT], 0);
  CHECK_GL_ERROR(true);
  
  // check FBO completeness
  if (!_isFBOComplete("post-processing"))
    return false;
  return true;
}

inline bool Core::_clearPostProcessing(const GLfloat * bgClear,
                                       const GLfloat * blackClear)
{
  // clear buffers
  glClearBufferfv(GL_COLOR, 0, bgClear);
  glClearBufferfv(GL_COLOR, 1, blackClear);
  glClear(GL_DEPTH_BUFFER_BIT);
  CHECK_GL_ERROR(true);
  if (!_swapPostProcessing())
    return false;
  glClearBufferfv(GL_COLOR, 0, bgClear);
  glClearBufferfv(GL_COLOR, 1, blackClear);
  glClear(GL_DEPTH_BUFFER_BIT);
  return true;
}
