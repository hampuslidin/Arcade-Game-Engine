//
//  core.cpp
//  Game Engine
//

#include "core.hpp"
#include <fstream>


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
const vector<vec3> & GraphicsComponent::vertexPositions() const
{
  return _vertexPositions;
}
const vector<vec3> & GraphicsComponent::vertexColors() const
{
  return _vertexColors;
}
const vector<ivec3> & GraphicsComponent::vertexIndices() const
{
  return _vertexIndices;
}

// MARK: Member functions
void GraphicsComponent::init(Entity * entity)
{
  Component::init(entity);
  
  // generate vertex array object
  glGenVertexArrays(1, &_vertexArrayObject);
  glBindVertexArray(_vertexArrayObject);
  
  // generate position buffer
  glGenBuffers(1, &_positionBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, _positionBuffer);
  glBufferData(GL_ARRAY_BUFFER,
               _vertexPositions.size()*sizeof(vec3),
               &_vertexPositions[0].x,
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
  glEnableVertexAttribArray(0);
  
  // generate color buffer
  glGenBuffers(1, &_colorBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, _colorBuffer);
  glBufferData(GL_ARRAY_BUFFER,
               _vertexColors.size()*sizeof(vec3),
               &_vertexColors[0].x,
               GL_STATIC_DRAW);
  glVertexAttribPointer(1, 3, GL_FLOAT, false, 0, 0);
  glEnableVertexAttribArray(1);
  
  // generate index buffer
  glGenBuffers(1, &_indexBuffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               _vertexIndices.size()*sizeof(ivec3),
               &_vertexIndices[0].x,
               GL_STATIC_DRAW);
  
  // TODO: handle errors
  // create shader program
  GLuint vertexShader   = glCreateShader(GL_VERTEX_SHADER);
  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  
  ifstream vertexShaderFile(_vertexShaderFilename);
  string vertexShaderSource((istreambuf_iterator<char>(vertexShaderFile)),
                            istreambuf_iterator<char>());
  
  ifstream fragmentShaderFile(_fragmentShaderFilename);
  string fragmentShaderSource((istreambuf_iterator<char>(fragmentShaderFile)),
                              istreambuf_iterator<char>());
  
  const char * vertexShaderSourceStr   = vertexShaderSource.c_str();
  const char * fragmentShaderSourceStr = fragmentShaderSource.c_str();
  
  glShaderSource(vertexShader, 1, &vertexShaderSourceStr, nullptr);
  glShaderSource(fragmentShader, 1, &fragmentShaderSourceStr, nullptr);
  glCompileShader(vertexShader);
  glCompileShader(fragmentShader);
  
  _shaderProgram = glCreateProgram();
  glAttachShader(_shaderProgram, fragmentShader);
  glAttachShader(_shaderProgram, vertexShader);
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
  glLinkProgram(_shaderProgram);
  
  // retrieve uniform locations
  _modelViewProjectionMatrixLocation =
    glGetUniformLocation(_shaderProgram, "modelViewProjectionMatrix");
}

void GraphicsComponent::render(const Core & core)
{
  // construct matrices
  const mat4 modelTranslation    = glm::translate(entity()->worldPosition());
  const mat4 modelRotation       = glm::toMat4(entity()->worldOrientation());
  const mat4 modelMatrix         = modelTranslation * modelRotation;
  const mat4 viewMatrix          = core.viewMatrix();
  const mat4 projectionMatrix    = core.projectionMatrix();
  mat4 modelViewProjectionMatrix = projectionMatrix * viewMatrix * modelMatrix;
  
  // send model-view-projection matrix to shader
  glUseProgram(_shaderProgram);
  glUniformMatrix4fv(_modelViewProjectionMatrixLocation,
                     1,
                     false,
                     &modelViewProjectionMatrix[0].x);
  
  // render
  glBindVertexArray(_vertexArrayObject);
  glDrawElements(GL_TRIANGLES,
                 (int)_vertexIndices.size()*sizeof(ivec3),
                 GL_UNSIGNED_INT,
                 0);
}

void GraphicsComponent::attachMesh(const vector<vec3> & positions,
                                   const vector<vec3> & colors,
                                   const vector<ivec3> & indices)
{
  _vertexPositions.resize(positions.size());
  _vertexColors.resize(colors.size());
  _vertexIndices.resize(indices.size());
  
  for (int i = 0; i < positions.size(); i++)
  {
    vec3 & vPos = _vertexPositions[i];
    vec3 pos = positions[i];
    vPos.x = pos.x;
    vPos.y = pos.y;
    vPos.z = pos.z;
  }
  
  for (int i = 0; i < colors.size(); i++)
  {
    vec3 & vCol = _vertexColors[i];
    vec3 col = colors[i];
    vCol.x = col.x;
    vCol.y = col.y;
    vCol.z = col.z;
  }
  
  for (int i = 0; i < indices.size(); i++)
  {
    ivec3 & vInd = _vertexIndices[i];
    vec3 ind = indices[i];
    vInd.x = ind.x;
    vInd.y = ind.y;
    vInd.z = ind.z;
  }
}

void GraphicsComponent::attachShader(const string & vertexShaderFilename,
                                     const string & fragmentShaderFilename)
{
  _vertexShaderFilename   = vertexShaderFilename;
  _fragmentShaderFilename = fragmentShaderFilename;
}

string GraphicsComponent::trait() const { return "Graphics"; }


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

const vec3 & Entity::localPosition() const    { return _localPosition; }
const quat & Entity::localOrientation() const { return _localOrientation; }
const vec3 & Entity::velocity() const         { return _velocity; }
const vec3 & Entity::force() const            { return _force; }

bool Entity::enabled() const { return _enabled; }

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
  , _localPosition(0.0f)
  , _localOrientation(1.0f, 0.0f, 0.0f, 0.0f)
  , _velocity(0.0f)
  , _force(0.0f)
  , _enabled(false)
  , _transformNeedsUpdating(true)
{}

void Entity::init(Core * core)
{
  _core = core;
  _enabled = true;
  
  if (_input)     _input->init(this);
  if (_animation) _animation->init(this);
  if (_collider)  _collider->init(this);
  if (_rigidBody) _rigidBody->init(this);
  if (_audio)     _audio->init(this);
  if (_graphics)  _graphics->init(this);
  
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
  if (_input)     _input->reset();
  if (_animation) _animation->reset();
  if (_collider)  _collider->reset();
  if (_rigidBody) _rigidBody->reset();
  if (_audio)     _audio->reset();
  if (_graphics)  _graphics->reset();
  
  for (auto child : _children) child->reset();
}

void Entity::destroy()
{
  for (auto child : _children)
  {
    child->destroy();
  }
  _children.clear();
  
  if (_input)     delete _input;
  if (_animation) delete _animation;
  if (_collider)  delete _collider;
  if (_rigidBody) delete _rigidBody;
  if (_audio)     delete _audio;
  if (_graphics)  delete _graphics;
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

mat4 Entity::localTransform()
{
  return localTranslation() * localRotation();
}

mat4 Entity::localTranslation()
{
  return glm::translate(_localPosition);
}

mat4 Entity::localRotation()
{
  return glm::toMat4(_localOrientation);
}

vec3 Entity::localUp()
{
  return glm::rotate(_localOrientation, Core::WORLD_UP);
}

vec3 Entity::localDown()
{
  return glm::rotate(_localOrientation, Core::WORLD_DOWN);
}

vec3 Entity::localLeft()
{
  return glm::rotate(_localOrientation, Core::WORLD_LEFT);
}

vec3 Entity::localRight()
{
  return glm::rotate(_localOrientation, Core::WORLD_RIGHT);
}

vec3 Entity::localForward()
{
  return glm::rotate(_localOrientation, Core::WORLD_FORWARD);
}

vec3 Entity::localBackward()
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

mat4 Entity::worldTransform()
{
  return worldTranslation() * worldRotation();
}

mat4 Entity::worldTranslation()
{
  return glm::translate(_worldPosition);
}

mat4 Entity::worldRotation()
{
  return glm::toMat4(worldOrientation());
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

void Entity::resetPositionX(float x)
{
  _localPosition.x = 0.0f;
  translate({x, 0.0f, 0.0f});
}

void Entity::resetPositionY(float y)
{
  _localPosition.y = 0.0f;
  translate({0.0f, y, 0.0f});
}

void Entity::resetPositionZ(float z)
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
    _worldPosition    = _parent->worldPosition() + _localPosition;
    _worldOrientation = _parent->worldOrientation() * _localOrientation;
  }
  else
  {
    _worldPosition    = _localPosition;
    _worldOrientation = _localOrientation;
  }
}


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
  auto windowOptions = SDL_WINDOW_SHOWN |
  SDL_WINDOW_RESIZABLE |
  SDL_WINDOW_FULLSCREEN_DESKTOP |
  SDL_WINDOW_ALLOW_HIGHDPI;
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
  
  // 1 for v-sync
  SDL_GL_SetSwapInterval(1);
  
  // initialize entities
  _root.init(this);
  _root.reset();
  
  // save references for colliders
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
  
  // record time
  static double prevTime = elapsedTime();
  double startTime = elapsedTime();
  _deltaTime = startTime - prevTime;
  prevTime = startTime;
  
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
  
  // set up OpenGl
  ivec2 d = viewDimensions();
  glViewport(0, 0, d.x, d.y);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  
  // clear screen
  glClearColor(_backgroundColor.r, _backgroundColor.g, _backgroundColor.b, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  // handle input for entities
  for (auto & entity : _entities)
  {
    if (entity.enabled() && entity.input())
      entity.input()->handleInput(*this);
  }
  
  // animate entities
  for (auto & entity : _entities)
  {
    if (entity.enabled() && entity.animation())
      entity.animation()->animate(*this);
  }
  
  // update colliders
  for (auto & entity : _entities)
  {
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
    const vec3 colliderVelocity  = colliderEntity->velocity();
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
  
  // update rigid bodies
  for (auto & entity : _entities)
  {
    if (entity.enabled() && entity.rigidBody())
      entity.rigidBody()->update(*this);
  }
  
  // update view and projection matrix
  mat4 cameraTranslation = glm::translate(_camera->worldPosition());
  cameraTranslation[3].x *= -1;
  cameraTranslation[3].y *= -1;
  cameraTranslation[3].z *= -1;
  quat inverseCameraOrientation = glm::inverse(_camera->worldOrientation());
  const mat4 cameraRotation = glm::toMat4(inverseCameraOrientation);
  _viewMatrix = cameraRotation * cameraTranslation;
  _projectionMatrix = perspective(radians(45.0f),
                                  float(d.x) / float(d.y),
                                  0.01f,
                                  1e38f);
  
  // update entities to their next state and render
  for (auto & entity : _entities)
  {
    if (entity.enabled() && entity.graphics())
      entity.graphics()->render(*this);
  }
  
  // swap buffers
  SDL_GL_SwapWindow(_window);
  
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

Entity * Core::createEntity(string id, string parentId)
{
  Entity * entity = nullptr;
  if (_entityCount < _maximumNumberOfEntities && !_root.findChild(id))
  {
    Entity * parent = &_root;
    if (parentId.compare("root") == 0 || (parent = _root.findChild(parentId)))
    {
      entity = &_entities[_entityCount++];
      entity->assignIdentifier(id);
      parent->addChild(entity);
    }
  }
  return entity;
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
    
#ifdef GAME_ENGINE_DEBUG
    printf("/**************** PAUSED ****************/\n");
#endif
    
  }
  else if (!_pause && pauseToggle)
  {
    pauseToggle = false;
    totalPauseDuration += elapsed - lastPauseTime;
    
#ifdef GAME_ENGINE_DEBUG
    printf("/**************** RESUMED ***************/\n");
#endif
    
  }
  
#ifdef GAME_ENGINE_DEBUG
  static double last_print_time;
  
  if (elapsed - last_print_time >= 0.1)
  {
    printf("Elapsed: %f\t\t", elapsed);
    printf("Effective elapsed: %f\t\t",
           !_pause
           ? elapsed - totalPauseDuration
           : lastPauseTime - totalPauseDuration);
    printf("Pause time: %f\t\t", lastPauseTime);
    printf("Pause duration: %f\n",
           !_pause
           ? totalPauseDuration
           : totalPauseDuration + elapsed - lastPauseTime);
    last_print_time = elapsed;
  }
#endif
  
  return (!_pause ? elapsed : lastPauseTime) - totalPauseDuration;
}

ivec2 Core::viewDimensions() const 
{
  ivec2 v;
  SDL_GL_GetDrawableSize(_window, &v.x, &v.y);
  return v;
}
