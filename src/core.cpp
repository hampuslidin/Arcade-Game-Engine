//
//  core.cpp
//  Game Engine
//

#include <iostream>
#include <fstream>

#include <imgui.h>
#include <imgui_impl_sdl_gl3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "core.hpp"


// MARK: Member functions
Sprite::Sprite(SDL_Renderer * renderer, SDL_Texture * texture)
  : _renderer(renderer)
  , _texture(texture)
{}

Sprite * Sprite::createSprite(SDL_Renderer * renderer, const char * filename)
{
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
      /*gluErrorString(err) <<*/ endl;
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
      !_createDeferredAmbientShader() || !_createDeferredNullShader() ||
      !_createDeferredLightShader() || !_createPostMotionBlurShader() ||
      !_createPostOutputShader())
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
  _motionBlurEnabled = false;
  _deferredEnabled   = false;
  _particlesEnabled  = false;
  _resetGUIParameters();

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

  // deferred shading data
  const int numLights = std::min((int)_lights.size(), _deferNumLights);
  const mat4 S        = glm::scale(vec3(_deferAttDist));

  // update view and projection matrix
  _prevViewMatrix           = _viewMatrix;
  _prevProjMatrix           = _projMatrix;
  mat4 cameraTranslation    = glm::translate(-_camera->worldPosition());
  const mat4 cameraRotation = glm::inverse(_camera->worldRotation());
  _viewMatrix               = cameraRotation * cameraTranslation;
  _projMatrix               = glm::perspective((float)M_PI/4.0f, (float)d.x/d.y,
                                               0.01f, 1000.0f);
  const mat4 PV             = _projMatrix * _viewMatrix;
  const mat4 prevPV         = _prevProjMatrix * _prevViewMatrix;

  // define clear colors
  const GLfloat bgClear[]    = {_bgColor.r, _bgColor.g, _bgColor.b, 1.0f};
  const GLfloat blackClear[] = {0.0f,       0.0f,       0.0f,       1.0f};

  // clear post-processing FBO
  // FIXME: why is clearing so slow?
  glBindFramebuffer(GL_FRAMEBUFFER, _postFBO);
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
        const mat4 M       = entity.worldTransform();
        const mat4 PVM     = PV * M;
        const mat4 prevPVM = prevPV * M;
        const mat3 N       = glm::inverse(mat3(M));

        // set shader uniforms
        glUseProgram(_deferSh.prog);
        glUniformMatrix4fv(_deferSh.locs["M"],       1, false, &M[0].x);
        glUniformMatrix4fv(_deferSh.locs["PVM"],     1, false, &PVM[0].x);
        glUniformMatrix4fv(_deferSh.locs["prevPVM"], 1, false, &prevPVM[0].x);
        glUniformMatrix3fv(_deferSh.locs["N"],       1, true,  &N[0].x);
        CHECK_GL_ERROR(true);

        // render
        graphics->render(*this);
        CHECK_GL_ERROR(true);
      }
    }
  }

  ////////////////////////////////////////
  // RENDERING - (DEFERRED) LIGHTING PASS
  ////////////////////////////////////////

  if (_deferredEnabled)
  {
    // disable depth testing and writing
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    // use ambient shader
    glBindFramebuffer(GL_FRAMEBUFFER, _postFBO);
    glUseProgram(_ambSh.prog);
    CHECK_GL_ERROR(true);

    // bind color texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _deferColMap);
    CHECK_GL_ERROR(true);

    // update shader uniform
    glUniform3fv(_ambSh.locs["ambCol"], 1, &_deferAmbCol.x);
    CHECK_GL_ERROR(true);

    // render ambient lighting
    glBindVertexArray(_quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    CHECK_GL_ERROR(true);

    // enable additive blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    CHECK_GL_ERROR(true);

    // enable stencil testing
    glEnable(GL_STENCIL_TEST);
    glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
    glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

    // set front-face for culling
    glCullFace(GL_FRONT);

    // render deferred lighting
    for (int i = 0; i < numLights; ++i)
    {
      Entity * light = _lights[i];
      const mat4 PVM = PV * light->worldTranslation() * S;
      _deferredStencilPass(light, PVM);
      _deferredLightingPass(light, PVM);
    }

    // revert to back-face for culling
    glCullFace(GL_BACK);
    CHECK_GL_ERROR(true);
  }

  ////////////////////////////////////////
  // RENDERING - ENTITIES
  ////////////////////////////////////////

  // disable blending and stencil testing
  glDisable(GL_BLEND);
  glDisable(GL_STENCIL_TEST);
  CHECK_GL_ERROR(true);

  // enable depth testing
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
  CHECK_GL_ERROR(true);

  // render entities using default shading
  int lightCount = 0;
  for (int i = 0; i < _entityCount; ++i)
  {
    auto & entity = _entities[i];
    auto graphics = entity.graphics();
    if (entity.enabled() && graphics && lightCount < numLights &&
        (!_deferredEnabled || !graphics->deferredShading() ||
         entity.type() == Light))
    {
      if (entity.type() == Light)
        ++lightCount;

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

  // enable transparecy blending
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  CHECK_GL_ERROR(true);

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
  CHECK_GL_ERROR(true);

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
    glBindTexture(GL_TEXTURE_2D, _postDepthStencilMap);
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

  // clear default frame buffer
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClearBufferfv(GL_COLOR, 0, bgClear);
  glClear(GL_DEPTH_BUFFER_BIT);
  CHECK_GL_ERROR(true);

  // bind textures
  glUseProgram(_postOutputSh.prog);
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
  ImGui::LabelText("Framerate", "%0.1f fps", frameRate);
  ImGui::LabelText("Resolution", "%ix%i", d.x, d.y);
  if (ImGui::Button("Reset"))
    _resetGUIParameters();
  ImGui::Separator();
  ImGui::Checkbox("Motion blur", &_motionBlurEnabled);
  if (_motionBlurEnabled)
  {
    ImGui::Indent();
    ImGui::RadioButton("Camera", &_motionBlurMode, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Per object", &_motionBlurMode, 1);
    ImGui::Spacing();
    ImGui::SliderFloat("Velocity scaling", &_motionVelScaling, 0.0f, 2.0f);
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
    ImGui::Checkbox("Show light effect area", &_deferShowLightArea);
    ImGui::ColorEdit3("Ambient color", &_deferAmbCol.r);
    ImGui::SliderInt("Number of lights", &_deferNumLights, 0,
                     (int)_lights.size());
    ImGui::Text("Light attenuation");
    ImGui::SliderFloat("Distance", &_deferAttDist, 0.0f, 50.0f);
    ImGui::SliderFloat("Linear term", &_deferAttLin, 0.0f, 1.0f);
    ImGui::SliderFloat("Quadratic term", &_deferAttQuad, 0.0f, 1.0f);
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
  SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                      SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

  // create window
  auto windowOptions = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN |
    SDL_WINDOW_ALLOW_HIGHDPI;
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
  glEnable(GL_PROGRAM_POINT_SIZE);
  glBlendEquation(GL_FUNC_ADD);

  // clear and swap frame
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

void _createUnitSphere(vector<float> & verts, int phiDiv = 12,
                       int thetaDiv = 24)
{
  // resize vertices vector
  verts.resize(18*thetaDiv*(phiDiv-1));
  int i = 0;

  // compute constant step values
  const float ps     = M_PI/phiDiv;
  const float ts     = 2.0f*M_PI/thetaDiv;
  const float tsHalf = ts*0.5f;

  // declare variables
  float a, aNext;
  float u, uNext;
  float t, offs;

  // create bottom cap
  aNext = sinf(ps);
  uNext = -cosf(ps);
  for (int n = 0; n < thetaDiv; ++n, i += 9)
  {
    t = n*ts;

    // v1
    verts[i]   = 0.0f;
    verts[i+1] = -1.0f;
    verts[i+2] = 0.0f;

    // v2
    verts[i+3] = aNext*cosf(t-tsHalf);
    verts[i+4] = uNext;
    verts[i+5] = aNext*sinf(t-tsHalf);

    // v2
    verts[i+6] = aNext*cosf(t+tsHalf);
    verts[i+7] = uNext;
    verts[i+8] = aNext*sinf(t+tsHalf);
  }

  // create middle
  offs = tsHalf;
  for (int n = 1; n < phiDiv-1; ++n)
  {
    a     = aNext;
    u     = uNext;
    aNext =  sinf((n+1)*ps);
    uNext = -cosf((n+1)*ps);
    offs  = tsHalf * (n % 2);
    for (int m = 0; m < thetaDiv; ++m, i += 18)
    {
      t = m*ts + offs;

      // triangle 1
      // v1
      verts[i]    = a*cosf(t);
      verts[i+1]  = u;
      verts[i+2]  = a*sinf(t);

      // v2
      verts[i+3]  = aNext*cosf(t-tsHalf);
      verts[i+4]  = uNext;
      verts[i+5]  = aNext*sinf(t-tsHalf);

      // v2
      verts[i+6]  = aNext*cosf(t+tsHalf);
      verts[i+7]  = uNext;
      verts[i+8]  = aNext*sinf(t+tsHalf);

      // triangle 2
      // v1
      verts[i+9]  = a*cosf(t);
      verts[i+10] = u;
      verts[i+11] = a*sinf(t);

      // v2
      verts[i+12] = a*cosf(t-ts);
      verts[i+13] = u;
      verts[i+14] = a*sinf(t-ts);

      // v2
      verts[i+15] = aNext*cosf(t-tsHalf);
      verts[i+16] = uNext;
      verts[i+17] = aNext*sinf(t-tsHalf);
    }
  }

  // create top cap
  a =  sinf((float)M_PI-ps);
  u = -cosf((float)M_PI-ps);
  for (int m = 0; m < thetaDiv; ++m, i += 9)
  {
    t = m*ts + offs;

    // v1
    verts[i]   = a*cosf(t-tsHalf);
    verts[i+1] = u;
    verts[i+2] = a*sinf(t-tsHalf);

    // v2
    verts[i+3] = 0.0f;
    verts[i+4] = 1.0f;
    verts[i+5] = 0.0f;

    // v2
    verts[i+6] = a*cosf(t+tsHalf);
    verts[i+7] = u;
    verts[i+8] = a*sinf(t+tsHalf);
  }
}

bool Core::_generateBuffers()
{
  const ivec2 d = viewDimensions();
  GLuint attachments[4] =
  {
    GL_COLOR_ATTACHMENT0,
    GL_COLOR_ATTACHMENT1,
    GL_COLOR_ATTACHMENT2,
    GL_COLOR_ATTACHMENT3
  };

  ////////////////////////////////////////
  // QUAD VERTEX ARRAY OBJECT
  ////////////////////////////////////////

  // generate vertex array object
  glGenVertexArrays(1, &_quadVAO);
  glBindVertexArray(_quadVAO);
  CHECK_GL_ERROR(true);

  // submit vertices to GPU
  vector<float> quadVerts =
  {
    -1.0f, -1.0f,
     1.0f, -1.0f,
    -1.0f,  1.0f,
     1.0f,  1.0f
  };
  GLuint quadVertsBuf;
  glGenBuffers(1, &quadVertsBuf);
  glBindBuffer(GL_ARRAY_BUFFER, quadVertsBuf);
  glBufferData(GL_ARRAY_BUFFER, quadVerts.size()*sizeof(float),
               &quadVerts[0], GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, false, 0, 0);
  glEnableVertexAttribArray(0);
  CHECK_GL_ERROR(true);

  ////////////////////////////////////////
  // SPHERE VERTEX ARRAY OBJECT
  ////////////////////////////////////////

  // generate vertex array object
  glGenVertexArrays(1, &_sphereVAO);
  glBindVertexArray(_sphereVAO);
  CHECK_GL_ERROR(true);

  // create sphere
  vector<float> sphereVerts;
  _createUnitSphere(sphereVerts);
  _numSphereVerts = (int)sphereVerts.size()/3;

  // submit vertices to GPU
  GLuint sphereVertssBuf;
  glGenBuffers(1, &sphereVertssBuf);
  glBindBuffer(GL_ARRAY_BUFFER, sphereVertssBuf);
  glBufferData(GL_ARRAY_BUFFER, sphereVerts.size()*sizeof(float),
               &sphereVerts[0], GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
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

  // generate depth-stencil texture
  glGenTextures(1, &_postDepthStencilMap);
  glBindTexture(GL_TEXTURE_2D, _postDepthStencilMap);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, d.x, d.y, 0,
               GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
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
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                         GL_TEXTURE_2D, _postDepthStencilMap, 0);
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
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D,
                         _postVelMap, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                         GL_TEXTURE_2D, _postDepthStencilMap, 0);
  CHECK_GL_ERROR(true);

  // attach textures
  glDrawBuffers(4, attachments);
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
    "PVM", "prevPVM", "colTexMap", "hasDiffTexMap", "diffCol"
  };
  return _createShader(_defaultSh, "shaders/default.vert",
                       "shaders/default.frag", ids);
}

bool Core::_createDeferredGeometryShader()
{
  const vector<string> ids = {"M", "PVM", "prevPVM", "N"};
  return _createShader(_deferSh, "shaders/deferred_geometry.vert",
                       "shaders/deferred_geometry.frag", ids);
}

bool Core::_createDeferredAmbientShader()
{
  return _createShader(_ambSh, "shaders/quad.vert",
                       "shaders/deferred_ambient.frag",
                       {"colTexMap", "ambCol"});
}

bool Core::_createDeferredNullShader()
{
  return _createShader(_stencilSh, "shaders/PVM.vert", "shaders/null.frag",
                       {"PVM"});
}

bool Core::_createDeferredLightShader()
{
  const vector<string> ids =
  {
    "PVM", "posTexMap", "normTexMap", "colTexMap", "showLightArea", "lightPos",
    "lightCol", "attLin", "attQuad"
  };
  bool success = _createShader(_lightSh, "shaders/PVM.vert",
                               "shaders/deferred_lighting.frag", ids);
  if (success)
  {
    // set texture uniforms
    glUseProgram(_lightSh.prog);
    glUniform1i(_lightSh.locs["posTexMap"],  0);
    glUniform1i(_lightSh.locs["normTexMap"], 1);
    glUniform1i(_lightSh.locs["colTexMap"], 2);
    CHECK_GL_ERROR(true);
  }

  return success;
}

bool Core::_createPostMotionBlurShader()
{
  const vector<string> ids =
  {
    "colTexMap", "velTexMap", "depthTexMap", "currToPrev", "fps", "mode",
    "velScaling", "adaptVarFPS", "adaptNumSamples", "prefNumSamples"
  };
  bool success = _createShader(_motionSh, "shaders/quad.vert",
                               "shaders/post_motion_blur.frag", ids);
  if (success)
  {
    // set texture uniforms
    glUseProgram(_motionSh.prog);
    glUniform1i(_motionSh.locs["colTexMap"],  0);
    glUniform1i(_motionSh.locs["velTexMap"],   1);
    glUniform1i(_motionSh.locs["depthTexMap"], 2);
    CHECK_GL_ERROR(true);
  }

  return success;
}

bool Core::_createPostOutputShader()
{
  return _createShader(_postOutputSh, "shaders/quad.vert",
                       "shaders/post_output.frag", {"colTexMap"});
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

inline void Core::_deferredStencilPass(const Entity * light, const mat4 & PVM)
{
  // enable depth testing
  glEnable(GL_DEPTH_TEST);

  // disable face culling
  glDisable(GL_CULL_FACE);

  // all fragments succeed, only depth test is relevant
  glStencilFunc(GL_ALWAYS, 0, 0);
  glClear(GL_STENCIL_BUFFER_BIT);

  // use stencil shader
  glUseProgram(_stencilSh.prog);

  // update shader uniform
  glUniformMatrix4fv(_stencilSh.locs["PVM"], 1, false, &PVM[0].x);

  // render
  glBindVertexArray(_sphereVAO);
  glDrawArrays(GL_TRIANGLES, 0, _numSphereVerts);

  CHECK_GL_ERROR(true);
}

inline void Core::_deferredLightingPass(const Entity * light, const mat4 & PVM)
{
  // disable depth testing
  glDisable(GL_DEPTH_TEST);
  CHECK_GL_ERROR(true);

  // enable face culling
  glEnable(GL_CULL_FACE);
  CHECK_GL_ERROR(true);

  // only allow fragments with a non-zero stencil value
  glStencilFunc(GL_NOTEQUAL, 0, 0xFF);

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
  glUniform1i(_lightSh.locs["showLightArea"], _deferShowLightArea);
  glUniform1f(_lightSh.locs["attLin"],   _deferAttLin);
  glUniform1f(_lightSh.locs["attQuad"],  _deferAttQuad);
  CHECK_GL_ERROR(true);

  // retrieve light data
  auto graphics  = light->graphics();
  const vec3 pos = light->worldPosition();
  const vec3 col = graphics ? graphics->diffuseColor() : vec3(1.0f);

  // update shader uniforms
  glUniformMatrix4fv(_lightSh.locs["PVM"], 1, false, &PVM[0].x);
  glUniform3fv(_lightSh.locs["lightPos"], 1, &pos.x);
  glUniform3fv(_lightSh.locs["lightCol"], 1, &col.x);
  CHECK_GL_ERROR(true);

  // render
  glBindVertexArray(_sphereVAO);
  glDrawArrays(GL_TRIANGLES, 0, _numSphereVerts);
  CHECK_GL_ERROR(true);
}

inline void Core::_resetGUIParameters()
{
  _motionBlurMode        = 0;
  _motionVelScaling      = 0.1f;
  _motionAdaptVarFPS     = true;
  _motionAdaptNumSamples = true;
  _motionPrefNumSamples  = 16;
  _deferShowLightArea    = false;
  _deferAmbCol           = {0.3f, 0.3f, 0.3f};
  _deferNumLights        = (int)_lights.size();
  _deferAttDist          = 7.0f;
  _deferAttLin           = 0.12f;
  _deferAttQuad          = 0.58f;
  _particleSpawnRate     = 128;
  _particleLifeTime      = 2.0f;
  _particleConeSize      = 0.3f;
  _particleVelocity      = 5.0f;
}
