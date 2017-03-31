//
//  core.cpp
//  Game Engine
//

#include "core.hpp"
#include <fstream>
#include <set>


//
// MARK: - Sprite
//

// MARK: Member functions

Sprite::Sprite(SDL_Renderer * renderer, SDL_Texture * texture)
  : _renderer(renderer)
  , _texture(texture)
{}

Sprite * Sprite::createSprite(SDL_Renderer * renderer, const char * filename)
{
  SDL_Surface * loaded_surface = IMG_Load(filename);
  if (!loaded_surface)
  {
    SDL_Log("IMG_Load: %s\n", IMG_GetError());
  }
  else
  {
    auto texture = SDL_CreateTextureFromSurface(renderer, loaded_surface);
    SDL_FreeSurface(loaded_surface);
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


//
// MARK: - SpriteCollection
//

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
  {
    return _sprites.at(id);
  }
  return nullptr;
}

void SpriteCollection::draw(string id, int x, int y, int w, int h, int scale)
{
  Sprite * sprite;
  if ((sprite = retrieve(id))) sprite->draw(x, y, w, h, scale);
}


//
// MARK: - NotificationCenter
//

void NotificationCenter::notify(Event event, GameObject & sender)
{
  for (auto pair : _instance()._blocks[event])
  {
    if (pair.second == nullptr || pair.second == &sender)
      pair.first(event);
  }
}

ObserverID NotificationCenter::observe(function<void(Event)> block,
                                       Event event,
                                       GameObject * sender)
{
  auto & blocksForEvent = _instance()._blocks[event];
  auto size = blocksForEvent.size();
  blocksForEvent.push_back({block, sender});
  return hash<string>{}(event.id() + to_string(size));
}

void NotificationCenter::unobserve(ObserverID id,
                                   Event event,
                                   GameObject * sender)
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

// MARK: Private member functions

NotificationCenter & NotificationCenter::_instance()
{
  static NotificationCenter instance;
  return instance;
}

//
// MARK: - Core
//

// MARK: Static properties

const vec3 Core::WORLD_UP       { 0.0f,  1.0f,  0.0f};
const vec3 Core::WORLD_DOWN     { 0.0f, -1.0f,  0.0f};
const vec3 Core::WORLD_LEFT     {-1.0f,  0.0f,  0.0f};
const vec3 Core::WORLD_RIGHT    { 1.0f,  0.0f,  0.0f};
const vec3 Core::WORLD_FORWARD  { 0.0f,  0.0f, -1.0f};
const vec3 Core::WORLD_BACKWARD { 0.0f,  0.0f,  1.0f};

// MARK: Member property functions

double Core::elapsedTime()
{
  return SDL_GetTicks() / 1000.f;
}

double Core::effectiveElapsedTime()
{
  static double last_pause_time;
  static double total_pause_duration;
  static bool pause_toggle;
  
  const double elapsed = elapsedTime();
  
  if (_pause && !pause_toggle)
  {
    pause_toggle = true;
    last_pause_time = elapsed;
    
#ifdef GAME_ENGINE_DEBUG
    printf("/**************** PAUSED ****************/\n");
#endif
    
  }
  else if (!_pause && pause_toggle)
  {
    pause_toggle = false;
    total_pause_duration += elapsed - last_pause_time;
    
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
           ? elapsed - total_pause_duration
           : last_pause_time - total_pause_duration);
    printf("Pause time: %f\t\t", last_pause_time);
    printf("Pause duration: %f\n",
           !_pause
           ? total_pause_duration
           : total_pause_duration + elapsed - last_pause_time);
    last_print_time = elapsed;
  }
#endif
  
  return (!_pause ? elapsed : last_pause_time) - total_pause_duration;
}

ivec2 Core::viewDimensions()
{
  ivec2 v;
  SDL_GL_GetDrawableSize(_window, &v.x, &v.y);
  return v;
}

// MARK: Member functions

Core::Core(int numberOfEntities)
  : mousePosition(ivec2(0, 0))
  , mouseMovement(ivec2(0, 0))
  , sampleRate(44100)
  , maxVolume(0.05)
  , pScale(1)
  , _entityCount(0)
  , _maximumNumberOfEntities(numberOfEntities)
  , _reset(false)
  , _pause(false)
{
  _entities.resize(_maximumNumberOfEntities);
  root().id("root");
  camera(createEntity("camera"));
  pBackgroundColor().x = 0.0;
  pBackgroundColor().y = 0.0;
  pBackgroundColor().z = 0.0;
}

bool Core::init(CoreOptions & options)
{
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
  root().init(this);
  root().reset();
  
  // initialize audio
  auto fill_stream = [](void * userdata, uint8_t * stream, int length)
  {
    Core * core         = (Core*)userdata;
    int16_t * stream16b = (int16_t*)stream;
    double maxVolume    = core->maxVolume();
    
    for (int i = 0; i < length/2; i++) stream16b[i] = 0;
    
    function<void(Entity*)> callbacks;
    callbacks = [maxVolume, stream16b, length, &callbacks](Entity * entity)
    {
      AudioComponent * audio = entity->pAudio();
      if (audio) audio->audioStreamCallback(maxVolume, stream16b, length/2);
      
      for (auto child : entity->children())
      {
        callbacks(child);
      }
    };
    
    callbacks(&core->root());
  };
  
  SDL_AudioSpec desired_audio_spec;
  
  desired_audio_spec.freq     = sampleRate();
  desired_audio_spec.format   = AUDIO_S16SYS;
  desired_audio_spec.channels = 1;
  desired_audio_spec.samples  = 2048;
  desired_audio_spec.callback = fill_stream;
  desired_audio_spec.userdata = this;
  
  SDL_OpenAudio(&desired_audio_spec, nullptr);
  
  SDL_PauseAudio(0);
  
  return true;
}

bool Core::update()
{
  bool should_continue = true;
  
  // record time
  static double prev_time;
  double start_time = elapsedTime();
  deltaTime(start_time - prev_time);
  prev_time = start_time;
  
  // check user input
  mouseMovement().x = 0;
  mouseMovement().y = 0;
  SDL_Event event;
  set<SDL_Keycode> keysToPress, keysToRelease;
  while (SDL_PollEvent(&event))
  {
    switch (event.type)
    {
      case SDL_QUIT:
        should_continue = false;
        break;
      case SDL_KEYDOWN:
        keysToPress.insert(event.key.keysym.sym);
        break;
      case SDL_KEYUP:
        keysToRelease.insert(event.key.keysym.sym);
        break;
      case SDL_MOUSEMOTION:
        mousePosition().x = event.motion.x;
        mousePosition().y = event.motion.y;
        mouseMovement().x = event.motion.xrel;
        mouseMovement().y = event.motion.yrel;
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
  auto d = viewDimensions();
  glViewport(0, 0, d.x, d.y);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  
  // clear screen
  glClearColor(pBackgroundColor().r,
               pBackgroundColor().g,
               pBackgroundColor().b,
               1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  // update entities (except rendering)
  uint8_t mask = !_pause ? 0b111111 : 0b000001;
  for (uint8_t i = 0b100000; i > 0b000001; i = i >>= 1)
  {
    for (auto entity : _entities)
    {
      entity.update(mask & i);
    }
  }
  
  // update view and projection matrix
  mat4 cameraTranslation = glm::translate(camera()->worldPosition());
  cameraTranslation[3].x *= -1;
  cameraTranslation[3].y *= -1;
  cameraTranslation[3].z *= -1;
  quat inverseCameraOrientation = glm::inverse(camera()->worldOrientation());
  const mat4 cameraRotation = glm::toMat4(inverseCameraOrientation);
  viewMatrix(cameraRotation * cameraTranslation);
  projectionMatrix(perspective(radians(45.0f),
                               float(d.x) / float(d.y),
                               0.01f,
                               300.0f));
  
  // render entities
  for (auto entity : _entities)
  {
    entity.update(0b000001);
  }
  
  // swap buffers
  SDL_GL_SwapWindow(_window);
  
  // possibly do a reset
  if (_reset)
  {
    _timers.clear();
    root().reset();
    _reset = false;
    resume();
  }
  
  // go through timers
  int i = 0;
  while (i < _timers.size())
  {
    auto pair = _timers[i];
    const double current_time = pair.second == _EFFECTIVE
    ? effectiveElapsedTime()
    : elapsedTime();
    if (current_time >= pair.first.endTime)
    {
      pair.first.block();
      _timers.erase(_timers.begin() + i);
    }
    else i++;
  }
  
  return should_continue;
}

void Core::destroy()
{
  root().destroy();
  
  SDL_CloseAudio();
  SDL_DestroyWindow(_window);
  SDL_Quit();
}

Entity * Core::createEntity(string id, string parentId)
{
  Entity * entity = nullptr;
  if (_entityCount < _maximumNumberOfEntities && !root().findChild(id))
  {
    Entity * parent = &root();
    if (parentId.compare("root") == 0 || (parent = root().findChild(parentId)))
    {
      entity = &_entities[_entityCount++];
      entity->id(id);
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

maybe<bool> Core::checkKey(string name)
{
  if (_keyControls.find(name) != _keyControls.end())
  {
    return maybe<bool>::just(_keyControls[name].second);
  }
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


//
// MARK: - Entity
//

// MARK: Member functions

Entity::Entity()
  : core(nullptr)
  , parent(nullptr)
  , pInput(nullptr)
  , pAnimation(nullptr)
  , pRigidBody(nullptr)
  , pAudio(nullptr)
  , pGraphics(nullptr)
  , pVelocity(vec3(0.0f))
  , localPosition(vec3(0.0f))
  , localOrientation(quat(1.0f, 0.0f, 0.0f, 0.0f))
  , _transformNeedsUpdating(true)
{}

void Entity::init(Core * core)
{
  this->core(core);
  pEnabled(true);
  
  if (pInput())     pInput()->init(this);
  if (pAnimation()) pAnimation()->init(this);
  if (pRigidBody()) pRigidBody()->init(this);
  if (pAudio())     pAudio()->init(this);
  if (pGraphics())  pGraphics()->init(this);
  
  if (parent() != nullptr)
  {
    auto eventHandler = [this](Event)
    {
      _transformNeedsUpdating = true;
      NotificationCenter::notify(DidUpdateTransform, *this);
    };
    NotificationCenter::observe(eventHandler, DidUpdateTransform, parent());
  }
  
  for (auto child : children()) child->init(core);
}

void Entity::reset()
{
  pVelocity() = vec3(0.0f);
  
  if (pInput())     pInput()->reset();
  if (pAnimation()) pAnimation()->reset();
  if (pRigidBody()) pRigidBody()->reset();
  if (pAudio())     pAudio()->reset();
  if (pGraphics())  pGraphics()->reset();
  
  for (auto child : children()) child->reset();
}

void Entity::destroy()
{
  for (auto child : children())
  {
    child->destroy();
  }
  children().clear();
  
  if (pInput())     delete pInput();
  if (pAnimation()) delete pAnimation();
  if (pRigidBody()) delete pRigidBody();
  if (pAudio())     delete pAudio();
  if (pGraphics())  delete pGraphics();
}

Dimension2 Entity::dimensions()
{
  return pGraphics() ? pGraphics()->bounds().dim : Dimension2 {};
}

void Entity::addChild(Entity * child)
{
  children().push_back(child);
  child->parent(this);
}

Entity * Entity::findChild(string id)
{
  for (auto child : children())
  {
    if (child->id().compare(id) == 0) return child;
    auto possible_find = child->findChild(id);
    if (possible_find) return possible_find;
  }
  return nullptr;
}

void Entity::removeChild(string id)
{
  for (int i = 0; i < children().size(); i++)
  {
    auto child = children()[i];
    if (child->id() == id)
    {
      child->parent(nullptr);
      children().erase(children().begin()+i);
    }
  }
}

mat4 Entity::localTransform()
{
  return localTranslation() * localRotation();
}

mat4 Entity::localTranslation()
{
  return glm::translate(localPosition());
}

mat4 Entity::localRotation()
{
  return glm::toMat4(localOrientation());
}

vec3 Entity::localUp()
{
  return glm::rotate(localOrientation(), Core::WORLD_UP);
}

vec3 Entity::localDown()
{
  return glm::rotate(localOrientation(), Core::WORLD_DOWN);
}

vec3 Entity::localLeft()
{
  return glm::rotate(localOrientation(), Core::WORLD_LEFT);
}

vec3 Entity::localRight()
{
  return glm::rotate(localOrientation(), Core::WORLD_RIGHT);
}

vec3 Entity::localForward()
{
  return glm::rotate(localOrientation(), Core::WORLD_FORWARD);
}

vec3 Entity::localBackward()
{
  return glm::rotate(localOrientation(), Core::WORLD_BACKWARD);
}

vec3 Entity::worldPosition()
{
  if (_transformNeedsUpdating) _updateTransform();
  return _worldPosition;
}

quat Entity::worldOrientation()
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
  return glm::translate(worldPosition());
}

mat4 Entity::worldRotation()
{
  return glm::toMat4(worldOrientation());
}

void Entity::translate(float dx, float dy, float dz)
{
  localPosition().x += dx;
  localPosition().y += dy;
  localPosition().z += dz;
  _transformNeedsUpdating = true;
  NotificationCenter::notify(DidUpdateTransform, *this);
}

void Entity::rotate(float angle, vec3 axis)
{
  localOrientation() = glm::angleAxis(angle, axis) * localOrientation();
  _transformNeedsUpdating = true;
  NotificationCenter::notify(DidUpdateTransform, *this);
}

void Entity::setPosition(float x, float y, float z)
{
  localPosition(vec3(0.0f));
  translate(x, y, z);
}
void Entity::setPositionX(float x)
{
  vec3 previousPosition = localPosition();
  translate(x, previousPosition.y, previousPosition.z);
}

void Entity::setPositionY(float y)
{
  vec3 previousPosition = localPosition();
  translate(previousPosition.x, y, previousPosition.z);
}

void Entity::setPositionZ(float z)
{
  vec3 previousPosition = localPosition();
  translate(previousPosition.x, previousPosition.y, z);
}

void Entity::setOrientation(float pitch, float yaw, float roll)
{
  quat qX = glm::angleAxis(pitch, Core::WORLD_RIGHT);
  quat qY = glm::angleAxis(yaw,   Core::WORLD_UP);
  quat qZ = glm::angleAxis(roll,  Core::WORLD_BACKWARD);
  localOrientation(qZ * qY * qX);
  _transformNeedsUpdating = true;
  NotificationCenter::notify(DidUpdateTransform, *this);
}

void Entity::setPitch(float pitch)
{
  vec3 eulerAngles = glm::eulerAngles(localOrientation());
  setOrientation(pitch, eulerAngles.y, eulerAngles.z);
}

void Entity::setYaw(float yaw)
{
  vec3 eulerAngles = glm::eulerAngles(localOrientation());
  setOrientation(eulerAngles.x, yaw, eulerAngles.z);
}

void Entity::setRoll(float roll)
{
  vec3 eulerAngles = glm::eulerAngles(localOrientation());
  setOrientation(eulerAngles.x, eulerAngles.y, roll);
}

void Entity::update(uint8_t component_mask)
{
  if (pEnabled())
  {
    if (component_mask & 0b100000 && pInput())
    {
      pInput()->update(*core());
    }
    if (component_mask & 0b010000 && pAnimation())
    {
      pAnimation()->update(*core());
    }
    if (component_mask & 0b001000 && pCollider())
    {
      pCollider()->update(*core());
    }
    if (component_mask & 0b000100 && pRigidBody())
    {
      pRigidBody()->update(*core());
    }
    if (component_mask & 0b000010 && pAudio())
    {
      pAudio()->update(*core());
    }
    if (component_mask & 0b000001 && pGraphics())
    {
      pGraphics()->update(*core());
    }
  }
}

void Entity::_updateTransform()
{
  if (parent())
  {
    _worldPosition = parent()->worldPosition() + localPosition();
    _worldOrientation = parent()->worldOrientation() * localOrientation();
  }
}

//
// MARK: - Component
//

// MARK: Property functions

string Component::id()
{
  string id = trait() + "Component";
  if (entity()) id = entity()->id() + id;
  return id;
}

// MARK: Member functions
void Component::init(Entity * entity)
{
  this->entity(entity);
}


//
// MARK: - InputComponent
//

// MARK: Property functions

string InputComponent::trait() { return "Input"; }


//
// MARK: - GraphicsComponent
//

// MARK: Property functions

string GraphicsComponent::trait() { return "Mesh"; }

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
               vertexPositions().size()*sizeof(vec3),
               &vertexPositions()[0].x,
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
  glEnableVertexAttribArray(0);
  
  // generate color buffer
  glGenBuffers(1, &_colorBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, _colorBuffer);
  glBufferData(GL_ARRAY_BUFFER,
               vertexColors().size()*sizeof(vec3),
               &vertexColors()[0].x,
               GL_STATIC_DRAW);
  glVertexAttribPointer(1, 3, GL_FLOAT, false, 0, 0);
  glEnableVertexAttribArray(1);
  
  // generate index buffer
  glGenBuffers(1, &_indexBuffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               vertexIndices().size()*sizeof(int),
               &vertexIndices()[0],
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

void GraphicsComponent::update(Core & core)
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
                 (int)vertexIndices().size(),
                 GL_UNSIGNED_INT,
                 0);
}

void GraphicsComponent::attachMesh(vector<vec3> & positions,
                                   vector<vec3> & colors,
                                   vector<int> & indices)
{
  vertexPositions().resize(positions.size());
  vertexColors().resize(colors.size());
  vertexIndices().resize(indices.size());
  
  for (int i = 0; i < positions.size(); i++)
  {
    vec3 & vPos = vertexPositions()[i];
    vec3 pos = positions[i];
    vPos.x = pos.x;
    vPos.y = pos.y;
    vPos.z = pos.z;
  }
  
  for (int i = 0; i < colors.size(); i++)
  {
    vec3 & vCol = vertexColors()[i];
    vec3 col = colors[i];
    vCol.x = col.x;
    vCol.y = col.y;
    vCol.z = col.z;
  }
  
  for (int i = 0; i < indices.size(); i++)
  {
    vertexIndices()[i] = indices[i];
  }
}

void GraphicsComponent::attachShader(string vertexShaderFilename,
                                     string fragmentShaderFilename)
{
  _vertexShaderFilename   = vertexShaderFilename;
  _fragmentShaderFilename = fragmentShaderFilename;
}

void GraphicsComponent::offsetTo(int x, int y)
{
  bounds().pos.x = x;
  bounds().pos.y = y;
}

void GraphicsComponent::offsetBy(int dx, int dy)
{
  bounds().pos.x += dx;
  bounds().pos.y += dy;
}

void GraphicsComponent::resizeTo(int w, int h)
{
  bounds().dim.x = w;
  bounds().dim.y = h;
}

void GraphicsComponent::resizeBy(int dw, int dh)
{
  bounds().dim.x += dw;
  bounds().dim.y += dh;
}
