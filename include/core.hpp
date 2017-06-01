//
//  core.hpp
//  Game Engine
//

#pragma once

#include <map>
#include <set>
#include <vector>
#include <string>
#include <functional>

#include <SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "types.hpp"

using namespace std;
using namespace glm;

#define CHECK_GL_ERROR(fatal) (Core::CheckGLError(fatal) && (__debugbreak(), 1))
#if !defined(_WIN32)
#	define __debugbreak() assert(false)
#endif

class Sprite;
class SpriteCollection;
class NotificationCenter;
class Timer;
class Synthesizer;
class Core;
class GameObject;
class Entity;
class Component;
class InputComponent;
class AnimationComponent;
class ColliderComponent;
class SphereColliderComponent;
class RigidBodyComponent;
class AudioComponent;
class ParticleSystemComponent;
class GraphicsComponent;

// MARK: Events

const Event DidStartAnimating("DidStartAnimating");
const Event DidStopAnimating("DidStopAnimating");
const Event DidCollide("DidCollide");
const Event DidMoveIntoView("DidMoveIntoView");
const Event DidMoveOutOfView("DidMoveOutOfView");
const Event DidUpdateTransform("DidUpdateTransform");


// MARK: -

/**
 *  Defines a sprite and methods for drawing it to a SDL rendering context.
 */
class Sprite
{

public:

  // MARK: Member functions

  Sprite(SDL_Renderer * renderer, SDL_Texture * texture);
  static Sprite * createSprite(SDL_Renderer * renderer, const char * filename);
  void destroy();
  void draw(int x, int y, int w, int h, int scale = 1);

private:
  SDL_Renderer * _renderer;
  SDL_Texture * _texture;
};


// MARK: -
/**
 *  Defines a collection of sprites.
 */
class SpriteCollection
{

public:
  // MARK: Member functions
  SpriteCollection(SpriteCollection const &) = delete;
  static SpriteCollection & main();
  void init(SDL_Renderer * renderer);
  Sprite * create(string id, const char * filename);
  void destroy(string id);
  void destroyAll();
  Sprite * retrieve(string id);
  void draw(string id, int x, int y, int w, int h, int scale = 1);
  void operator=(SpriteCollection const &) = delete;

private:
  // MARK: Private
  SDL_Renderer * _renderer;
  map<string, Sprite*> _sprites;

  SpriteCollection() {};

};


// MARK: -
typedef size_t ObserverID;
class NotificationCenter
{

public:
  // MARK: Types
  typedef size_t ObserverID;

  // MARK: Member functions
  static void notify(Event event, GameObject & sender);
  static ObserverID observe(function<void(Event)> block,
                            Event event,
                            const GameObject * sender);
  static void unobserve(ObserverID id,
                        Event event,
                        const GameObject * sender);

private:
  // MARK: Private
  map<Event, vector<pair<function<void(Event)>, const GameObject*>>> _blocks;

  NotificationCenter() {};
  static NotificationCenter & _instance();

};


// MARK: -
class Synthesizer
{

public:
  // MARK: Properties
  int bitRate() const;
  int sampleRate() const;

  // MARK: Member functions
  Synthesizer(int bitRate = 8, int sampleRate = 44100);
  void load(const char * filename);
  void select(string id);
  bool generate(int16_t * stream,
                int length,
                int & frame,
                double maxVolume,
                double duration,
                double fadeIn,
                double fadeOut);

private:
  // MARK: Private
  enum _WaveType
  {
    SMOOTH,
    TRIANGLE,
    SAWTOOTH,
    SQUARE
  };
  enum _PitchGlideType
  {
    LINEAR,
    EXPONENTIAL,
    LOGARITHMIC,
    INV_LOGARITHMIC
  };
  struct _Operator
  {
    double frequency               = 440;
    double modulationIndex         = 1.0;
    _WaveType waveType             = SMOOTH;
    double thresholdLow            = -1.0;
    double thresholdHigh           = 1.0;
    maybe<double> pitchGlide       = maybe<double>::nothing();
    _PitchGlideType pitchGlideType = EXPONENTIAL;
    vector<_Operator*> modulators  = {};
  };
  struct _Algorithm
  {
    vector<_Operator> operators;
    int numCarriers;
  };

  int _bitRate;
  int _sampleRate;
  map<string, _Algorithm> _algorithms;
  _Algorithm * _currentAlgorithm;

  double _calculateSample(_Operator op, double time, double duration);
  double _calculatePhase(_Operator op, double time, double duration);

};


// MARK: -
class GameObject {

public:
  // MARK: Properties
  const string & id() const;

  // MARK: Member functions
  void assignIdentifier(const string & id);

private:
  string _id;

};


// MARK: -
/**
 *  An abstrct class for a generic component.
 */
class Component
  : public GameObject
{

public:
  // MARK: Member functions
  virtual ~Component() {};
  virtual void init(Entity * entity);
  virtual void reset() {};
  string id();
  virtual string trait() const = 0;

protected:
  // MARK: Protected
  Entity * entity();

private:
  mutable Entity * _entity;

};


// MARK: -
/**
 *  InputConponent is responsible for defining the behavior of an Entity.
 */
class InputComponent
  : public Component
{

public:
  // MARK: Member functions
  virtual void handleInput(const Core & core) = 0;
  string trait() const;

};


// MARK: -
/**
 *  AnimationComponent is responsible for moving an Entity according to a path,
 *  either in local space or in world space.
 */
class AnimationComponent
  : public Component
{

public:
  // MARK: Types
  typedef vector<pair<vec3, vec3>> CubicHermiteCurve;
  typedef pair<pair<vec3, vec3>, pair<vec3, vec3>> CubicHermiteSpline;

  // MARK: Properties
  bool animating() const;
  const vec3 & endVelocity() const;

  // MARK: Member functions
  virtual void reset();
  virtual void animate(const Core & core);
  void addSegment(string id, vec3 position, vec3 velocity);
  void removeCurve(string id);

  /**
   *  Initiates an animation, which will get updated by the *update* member
   *  function.
   *
   *  @return 0 on success, 1 if animation with associated id does not exist.
   */
  void performAnimation(string id,
                        double duration,
                        bool updateVelocity = false);

  string trait() const;

private:
  bool _animating;
  vec3 _endVelocity;
  map<string, CubicHermiteCurve> _curves;
  CubicHermiteCurve _currentCurve;
  vec3 _startPosition;
  double _startTime;
  double _duration;
  bool _updateVelocity;

};


// MARK: -
/**
 *  ColliderComponent is an interface all colliders must inherit from.
 */
class ColliderComponent
  : public Component
{

public:
  // MARK: Properties
  virtual const box & staticAxisAlignedBoundingBox() const = 0;
  virtual const box & dynamicAxisAlignedBoundingBox() const = 0;
  const vec3 & origin() const;

  // MARK: Member functions
  ColliderComponent(const vec3 & origin = {0.0f, 0.0f, 0.0f});
  virtual void update(const Core & core) = 0;
  void reposition(const vec3 & origin);

private:
  vec3 _origin;

};


// MARK: -
/**
 *  SphereColliderComponent does collision detection for spheres.
 */
class SphereColliderComponent
  : public ColliderComponent
{

public:
  // MARK: Properties
  const box & staticAxisAlignedBoundingBox() const;
  const box & dynamicAxisAlignedBoundingBox() const;
  float radius() const;

  // MARK: Member functions
  SphereColliderComponent(float radius);
  SphereColliderComponent(const vec3 & origin, float radius);
  void update(const Core & core);
  void resize(float radius);
  string trait() const;

private:
  box _staticAxisAlignedBoundingBox;
  box _dynamicAxisAlignedBoundingBox;
  float _radius;

};


// MARK: -
/**
 *  RigidBodyComponent is responsible for updating the position of an Entity
 *  object, w.r.t. the laws of physics.
 */
class RigidBodyComponent
  : public Component
{

public:
  // MARK: Properties
  float mass() const;
  float thermalVelocity() const;
  const vec3 & gravity() const;
  bool kinematic() const;

  // MARK: Member functions
  RigidBodyComponent();
  virtual void init(Entity * entity);
  virtual void update(const Core & core);

  void setMass(float m);
  void setThermalVelocity(float v);
  void setGravity(float gx, float gy, float gz);
  void setKinematic(bool enabled);

  string trait() const;

private:
  float _mass;
  float _thermalVelocity;
  vec3 _gravity;
  bool _kinematic;

  bool _shouldSimulate;

};


// MARK: -
/**
 *  AudioComponent is responsible for generating and playing sounds.
 */
class AudioComponent
  : public Component
{
  friend Core;

public:
  // MARK: Member functions
  virtual void init(Entity * entity);
  string trait() const;

protected:
  // MARK: Protected
  Synthesizer & synthesizer();

  void playSound(string id,
                 double duration,
                 double fade_in = 0.01,
                 double fade_out = 0.01);

private:
  // MARK: Private
  struct _Audio
  {
    string id;
    double duration;
    double fadeIn;
    double fadeOut;
    int frame;
  };
  vector<_Audio> _audioPlayback;

  Synthesizer _synthesizer;

  void _audioStreamCallback(double maxVolume,
                            int16_t * stream,
                            int length);

};


// MARK: -
enum TextureType { Diffuse, Specular };


// MARK: -
/**
 *  GraphicsComponent is responsible for drawing an Entity to a SDL rendering
 *  context.
 */
class GraphicsComponent
  : public Component
{

public:
  // MARK: Properties
  const vector<float> & vertices() const;
  int numberOfVertices() const;
  bool hasDiffuseTexture() const;
  const vec3 & diffuseColor() const;
  bool deferredShading() const;

  // MARK: Member functions
  GraphicsComponent();
  virtual void init(Entity * entity);
  virtual void render(const Core & core);
  bool loadObject(const char * objectFileName);
  bool loadTexture(const char * textureFileName,
                   TextureType textureType);
  bool loadShader(const char * vertexShaderFileName,
                  const char * fragmentShaderFileName);

  string trait() const;

  void diffuseColor(const vec3 & color);
  void deferredShading(bool enabled);

private:
  vector<float> _verts;       // vertices array

  bool _hasDiffTex;           // diffuse texture flag
  vec3 _diffCol;              // diffuse color

  bool _deferShading;         // deferred shading flag

  GLuint _vao;                // vertex array object
  GLuint _colTexMap;         // diffuse texture map
  GLint  _prevMLoc, _MLoc;    // current and previous model matrix locations
  GLuint _VLoc, _PLoc, _NLoc; // view, projection and normal matrix locations
  GLuint _diffColLoc;         // diffuse color location

};


// MARK: -
class ParticleSystemComponent
  : public Component
{

public:
  // MARK: Properties
  int numberOfParticles() const;

  // MARK: Member functions
  ParticleSystemComponent(int numberOfParticles);
  virtual void init(Entity * entity);
  virtual void render(const Core & core);
  bool loadTexture(const char * textureFileName);
  bool loadShader(const char * vertexShaderFileName,
                  const char * fragmentShaderFileName);
  string trait() const;

private:
  struct _Particle
  {
    float age;
    float lifeTime;
    vec3 position;
    vec3 velocity;
  };

  int               _maxNumberOfParticles;
  vector<_Particle> _particles;
  vector<vec4>      _particleRenderData;

  GLuint _vertexArrayObject;
  GLuint _particleDataBuffer;
  GLuint _shaderProgram;
  GLuint _diffuseMap;
  GLuint _projMatrixLocation;
  GLuint _screenWidthLocation;
  GLuint _screenHeightLocation;

};


// MARK: -
enum EntityType { Default, Camera, Light };

// MARK: -
/**
 *  Defines a class that represents a game entity that resides in a game world.
 */
class Entity
  : public GameObject
{

public:
  // MARK: Properties
  const Core * core() const;
  const Entity * parent() const;
  const vector<Entity*> & children() const;

  InputComponent * input() const;
  AnimationComponent * animation() const;
  ColliderComponent * collider() const;
  RigidBodyComponent * rigidBody() const;
  AudioComponent * audio() const;
  GraphicsComponent * graphics() const;
  ParticleSystemComponent * particleSystem() const;

  const mat4 & previousWorldTransform() const;
  const vec3 & localPosition() const;
  const quat & localOrientation() const;
  const vec3 & localScale() const;
  const vec3 & velocity() const;
  const vec3 & force() const;

  bool enabled() const;
  const EntityType & type() const;

  // MARK: Member functions
  Entity();

  /**
   *  Initializes an entity.
   *
   *  If deriving classes override this method, it must call the base class
   *  method. This method calls itself on the entities children, so make sure
   *  to add all children before calling the base class method.
   *
   *  @param  core   The engine core.
   */
  virtual void init(Core * core);

  virtual void reset();

  /**
   *  Destroys an entity.
   *
   *  If deriving classes override this method, it must call the base class
   *  method. This method calls itself on the entities children, so make sure
   *  to do all children related operations before calling the base class
   *  method.
   */
  virtual void destroy();

  void nextFrame();

  /**
   *  Adds a child.
   *
   *  @param  child   A reference to an existing Entity.
   */
  void addChild(Entity * child);
  Entity * findChild(string id);
  void removeChild(string id);

  void attachInputComponent(InputComponent * input);
  void attachAnimationComponent(AnimationComponent * animation);
  void attachColliderComponent(ColliderComponent * collider);
  void attachRigidBodyComponent(RigidBodyComponent * rigidBody);
  void attachAudioComponent(AudioComponent * audio);
  void attachGraphicsComponent(GraphicsComponent * graphics);
  void attachParticleSystemComponent(ParticleSystemComponent * particleSystem);

  mat4 localTransform() const;
  mat4 localTranslation() const;
  mat4 localRotation() const;
  vec3 localUp() const;
  vec3 localDown() const;
  vec3 localLeft() const;
  vec3 localRight() const;
  vec3 localForward() const;
  vec3 localBackward() const;

  const vec3 & worldPosition() const;
  const quat & worldOrientation() const;
  const vec3 & worldScale() const;
  mat4 worldTransform() const;
  mat4 worldTranslation() const;
  mat4 worldRotation() const;
  mat4 worldScaling() const;

  void translate(const vec3 & d);
  void rotate(float angle, const vec3 & axis);
  void scale(float s);
  void scale(const vec3 & s);
  void accelerate(const vec3 & v);
  void applyForce(const vec3 & f);

  void reposition(const vec3 & p = {});
  void repositionX(float x);
  void repositionY(float y);
  void repositionZ(float z);

  void reorient(const vec3 & o = {});
  void resetPitch(float pitch);
  void resetYaw(float yaw);
  void resetRoll(float roll);

  void rescale(const vec3 & s = {1.0f, 1.0f, 1.0f});
  void rescaleX(float x);
  void rescaleY(float y);
  void rescaleZ(float z);

  void resetVelocity(const vec3 & v = {});
  void resetVelocityX(float vx);
  void resetVelocityY(float vy);
  void resetVelocityZ(float vz);

  void resetForce(const vec3 & f = {});
  void resetForceX(float fx);
  void resetForceY(float fy);
  void resetForceZ(float fz);

  void type(const EntityType & newType);

  bool operator ==(Entity & entity);
  bool operator !=(Entity & entity);

private:
  Core *          _core;
  Entity *        _parent;
  vector<Entity*> _children;

  InputComponent *          _input;
  AnimationComponent *      _animation;
  ColliderComponent *       _collider;
  RigidBodyComponent *      _rigidBody;
  AudioComponent *          _audio;
  ParticleSystemComponent * _particleSystem;
  GraphicsComponent *       _graphics;

  vec3 _localPosition;
  quat _localOrientation;
  vec3 _localScale;
  vec3 _velocity;
  vec3 _force;
  mat4 _previousWorldTransform;

  mutable vec3 _worldPosition;
  mutable quat _worldOrientation;
  mutable vec3 _worldScale;

  mutable bool _transformInvalid;

  bool _enabled;
  EntityType _type;

  void _updateTransform() const;

};

// MARK: -
class PointLight
  : public Entity
{

public:
  PointLight(float distance);

private:
  float _linear;
  float _quadratic;
};


// MARK: -
struct CoreOptions
{
  const char * title;
  int width;
  int height;
};


//
// MARK: - Core
//

/**
 *  Defines the core engine and is responsible for reading user input
 *  and updating all entities.
 */
class Core
{

public:
  // MARK: Properties
  const Entity & root() const;
  Entity * camera() const;

  double deltaTime() const;
  bool mouseClick() const;
  const ivec2 & mousePosition() const;
  const ivec2 & mouseMovement() const;

  int sampleRate() const;
  double maxVolume() const;

  const mat4 & previousViewMatrix() const;
  const mat4 & previousProjectionMatrix() const;
  const mat4 & viewMatrix() const;
  const mat4 & projectionMatrix() const;

  int scale() const;
  const vec3 & backgroundColor() const;

  int particleSpawnRate() const;
  float particleLifeTime() const;
  float particleConeSize() const;
  float particleVelocity() const;

  static constexpr float UNITS_PER_METER = 1.0f;
  static const vec3 WORLD_UP;
  static const vec3 WORLD_DOWN;
  static const vec3 WORLD_LEFT;
  static const vec3 WORLD_RIGHT;
  static const vec3 WORLD_FORWARD;
  static const vec3 WORLD_BACKWARD;

  // MARK: Class functions
  static float uniformRandom(float from, float to);

  /**
   *  Computes the intersection between two static AABBs.
   *
   *  The intersection between the two AABBs is stored in the parameter
   *  'intersection' only if there is an overlap. Otherwise, this parameter
   *  remains unchanged.
   *
   *  @param    a             The first static AABB.
   *  @param    b             The second static AABB.
   *  @param    intersection  The intersected volume.
   *  @returns  true, if the AABBs overlap, otherwise false.
   **/
  static bool AABBIntersect(const box & a,
                            const box & b,
                            box & intersection);

  /**
   *  Computes the intersection between two static spheres.
   *
   *  The intersection between the two spheres is stored in the parameter
   *  'intersection' only if there is an overlap. Otherwise, this parameter
   *  remains unchanged.
   *
   *  @param    o1            The origin of the first sphere.
   *  @param    o2            The origin of the second sphere.
   *  @param    r1            The radius of the first sphere.
   *  @param    r2            The radius of the second sphere.
   *  @param    intersection  The intersected distance and direction relative
   *                          the first sphere.
   *  @returns  true, if the spheres overlap, otherwise false.
   **/
  static bool SphereIntersect(const vec3 & o1,
                              const vec3 & o2,
                              float r1,
                              float r2,
                              vec3 & intersection);

  /**
   *  Computes the collision between two dynamic spheres.
   *
   *  The locations of the spheres at the point of collision, as well as the
   *  time the collision will occur, is stored in the parameters 'p1', 'p2', and
   *  'time', respectively, only if a collision will occur during the time of
   *  the frame. Otherwise, these parameters remain unchanged.
   *
   *  @param    o1            The origin of the first sphere.
   *  @param    o2            The origin of the second sphere.
   *  @param    v1            The velocity of the first sphere.
   *  @param    v2            The velocity of the second sphere.
   *  @param    r1            The radius of the first sphere.
   *  @param    r2            The radius of the second sphere.
   *  @param    time          The time when the spheres collide.
   *  @param    p1            The position of the first sphere at the point of
   *                          impact.
   *  @param    p2            The position of the second sphere at the point of
   *                          impact.
   *  @returns  true, if the spheres collide during the frame, otherwise false.
   **/
  static bool SphereCollision(const vec3 & o1,
                              const vec3 & o2,
                              const vec3 & v1,
                              const vec3 & v2,
                              float r1,
                              float r2,
                              float & time,
                              vec3 & p1,
                              vec3 & p2);

  static maybe<GLuint> CreateShaderProgram(const char * vertexShaderFileName,
                                           const char * fragmentShaderFileName);

  static bool CheckGLError(bool fatal = true);

  // MARK: Member functions
  Core(int numberOfEntities);
  bool init(const CoreOptions & options);
  bool update();
  void destroy();

  /**
   *  Creates and adds an Entity to the game world.
   *
   *  To attach an Entity to the game world, a unique *id* must be provided for
   *  the Entity to be created. By default, its parent is the **root** Entity.
   *  An optional *parentId* can be provided if the new Entity should belong to
   *  another, already existing Entity in the game world.
   *
   *  @param  id        The identifier for the Entity to be created.
   *  @param  parentId  The identifier for the parent Entity.
   *  @return A pointer to the newly created Entity. If the *id* is already
   *          associated with an Entity or if there is no Entity associated with
   *          the *parentId*, then *nullptr* is returned.
   */
  Entity * createEntity(const string & id,
                        const string & parentId = "root",
                        EntityType type = Default);

  const Entity * findEntity(const string & id) const;

  void createEffectiveTimer(double duration, function<void()> block);
  void createAccumulativeTimer(double duration, function<void()> block);
  void addControl(string name, int key);
  void removeControl(string name);
  maybe<bool> checkKey(string name) const;
  void reset(double after_duration = 0);
  void pause();
  void resume();
  void changeBackgroundColor(float r, float g, float b);

  double elapsedTime() const;
  double effectiveElapsedTime() const;
  ivec2 viewDimensions() const;

private:
  // MARK: Private types
  typedef map<string, pair<int, bool>> _KeyControls;
  struct _Timer
  {
    double endTime;
    function<void(void)> block;
  };
  enum _TimerType { _EFFECTIVE, _ACCUMULATIVE };
  struct _IntervalBound
  {
    enum Type { START, END };

    Type type;
    int i;
    const float * v;
  };
  struct _Shader
  {
    GLuint              prog;
    map<string, GLuint> locs;
  };

  Entity   _root;
  Entity * _camera;

  double _deltaTime;
  bool   _mouseClick;
  ivec2  _mousePosition;
  ivec2  _mouseMovement;

  int    _sampleRate;
  double _maxVolume;

  _Shader _defaultSh;
  _Shader _deferSh, _ambSh, _stencilSh, _lightSh;
  _Shader _motionSh, _postOutputSh;
  int     _numSphereVerts;
  GLuint  _quadVAO, _sphereVAO;
  GLuint  _deferFBO, _postFBO;
  GLuint  _deferPosMap, _deferNormMap, _deferColMap;
  GLuint  _postColMap[2], _postVelMap, _postDepthStencilMap;
#define POST_IN  0
#define POST_OUT 1

  mat4 _prevViewMatrix, _viewMatrix;
  mat4 _prevProjMatrix, _projMatrix;

  int  _scale;
  vec3 _bgColor;

  SDL_Window *  _window;
  SDL_GLContext _context;

  int             _entityCount;
  int             _maximumNumberOfEntities;
  vector<Entity>  _entities;
  vector<Entity*> _lights;
  vector<Entity*> _colliders;

  _KeyControls _keyControls;
  vector<pair<_Timer, _TimerType>> _timers;

  vector<_IntervalBound> _intervalBounds[3];
  vector<vector<bool>>   _intervalPairOverlaps[3];

  double _pauseDuration;
  bool _reset;
  bool _pause;

  bool  _motionBlurEnabled;
  int   _motionBlurMode;
  float _motionVelScaling;
  bool  _motionAdaptVarFPS;
  bool  _motionAdaptNumSamples;
  int   _motionPrefNumSamples;
  bool  _deferredEnabled;
  bool  _deferShowLightArea;
  vec3  _deferAmbCol;
  int   _deferNumLights;
  float _deferAttDist;
  float _deferAttLin;
  float _deferAttQuad;
  bool  _particlesEnabled;
  int   _particleSpawnRate;
  float _particleLifeTime;
  float _particleConeSize;
  float _particleVelocity;

  bool _initFrameworks(const char * title, int scrnW, int scrnH);
  bool _generateBuffers();
  bool _createShader(_Shader & sh, const char * vsfn, const char * fsfn,
                     const vector<string> & ids);
  bool _createDefaultShader();
  bool _createDeferredGeometryShader();
  bool _createDeferredAmbientShader();
  bool _createDeferredNullShader();
  bool _createDeferredLightShader();
  bool _createPostMotionBlurShader();
  bool _createPostOutputShader();
  inline bool _swapPostProcessing();
  inline bool _clearPostProcessing(const GLfloat * bgClear,
                                   const GLfloat * blackClear);
  inline void _deferredStencilPass(const Entity * light, const mat4 & PVM);
  inline void _deferredLightingPass(const Entity * light, const mat4 & PVM);
  inline void _resetGUIParameters();
};
