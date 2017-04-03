//
//  core.hpp
//  Game Engine
//

#pragma once

#include <map>
#include <vector>
#include <string>
#include <functional>
#include "types.hpp"

#ifdef __APPLE__
# include <SDL2/SDL.h>
# include <SDL2/SDL_audio.h>
# include <SDL2_image/SDL_image.h>
#elif defined(_WIN32)
# include "SDL.h"
# include "SDL_audio.h"
# include "SDL_image.h"
#endif

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

using namespace std;
using namespace glm;

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
class RigidBodyComponent;
class AudioComponent;
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
  virtual void update(const Core & core) {};
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
  virtual void update(const Core & core);
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
 *  ColliderComponent is responsible for setting the bounds for collision 
 *  detection between two colliders.
 */
class ColliderComponent
  : public Component
{
  
public:
  // MARK: Member functions
  virtual void update(const Core & core);
  virtual void collide(const ColliderComponent & obsticle,
                       const vec3 & colliderPosition,
                       const vec3 & obsticlePosition) const = 0;
  
private:
  bool _didCollide;
  
};


// MARK: -
/**
 *  AABBColliderComponent does collision detection for axis-aligned bounding 
 *  boxes.
 */
class AABBColliderComponent
  : public ColliderComponent
{
  
public:
  // MARK: Properties
  const box & collisionBox() const;
  
  // MARK: Member functions
  void collide(const ColliderComponent & obsticle,
               const vec3 & colliderPosition,
               const vec3 & obsticlePosition) const;
  void resizeCollisionBox(const vec3 & min, const vec3 & max);
  string trait() const;
  
private:
  box _collisionBox;
  
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
  const vec3 & gravity() const;
  bool kinematic() const;
  
  // MARK: Member functions
  RigidBodyComponent();
  virtual void init(Entity * entity);
  virtual void update(const Core & core);
  void setGravity(const vec3 & force);
  void setKinematic(bool enabled);
  string trait() const;
  
private:
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
  virtual void update(const Core & core) {};
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
/**
 *  GraphicsComponent is responsible for drawing an Entity to a SDL rendering
 *  context.
 */
class GraphicsComponent
  : public Component
{
  
public:
  // MARK: Properties
  const vector<vec3> & vertexPositions() const;
  const vector<vec3> & vertexColors() const;
  const vector<ivec3> & vertexIndices() const;
  
  // MARK: Member functions
  virtual void init(Entity * entity);
  virtual void update(const Core & core);
  void attachMesh(const vector<vec3> & positions,
                  const vector<vec3> & colors,
                  const vector<ivec3> & indices);
  void attachShader(string vertexShaderFilename, string fragmentShaderFilename);
  string trait() const;
  
private:
  vector<vec3>  _vertexPositions;
  vector<vec3>  _vertexColors;
  vector<ivec3> _vertexIndices;
  
  GLuint _vertexArrayObject;
  GLuint _positionBuffer;
  GLuint _colorBuffer;
  GLuint _indexBuffer;
  
  GLuint _shaderProgram;
  string _vertexShaderFilename;
  string _fragmentShaderFilename;

  GLint _modelViewProjectionMatrixLocation;
  
};


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
  
  const InputComponent * input() const;
  const AnimationComponent * animation() const;
  const ColliderComponent * collider() const;
  const RigidBodyComponent * rigidBody() const;
  const AudioComponent * audio() const;
  const GraphicsComponent * graphics() const;
  
  const vec3 & localPosition() const;
  const quat & localOrientation() const;
  
  const vec3 & velocity() const;
  bool enabled() const;
  
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
  
  mat4 localTransform();
  mat4 localTranslation();
  mat4 localRotation();
  vec3 localUp();
  vec3 localDown();
  vec3 localLeft();
  vec3 localRight();
  vec3 localForward();
  vec3 localBackward();
  
  const vec3 & worldPosition() const;
  const quat & worldOrientation() const;
  mat4 worldTransform();
  mat4 worldTranslation();
  mat4 worldRotation();
  
  void translate(float dx, float dy, float dz);
  void translate(float distance, const vec3 & direction);
  void rotate(float angle, vec3 axis);
  
  void reposition(float x, float y, float z);
  void setX(float x);
  void setY(float y);
  void setZ(float z);
  
  void reorient(float pitch, float yaw, float roll);
  void setPitch(float pitch);
  void setYaw(float yaw);
  void setRoll(float roll);
  
  void update(unsigned int componentMask);
  
  bool operator ==(Entity & entity);
  bool operator !=(Entity & entity);
  
private:
  Core *          _core;
  Entity *        _parent;
  vector<Entity*> _children;
  
  InputComponent *     _input;
  AnimationComponent * _animation;
  ColliderComponent *  _collider;
  RigidBodyComponent * _rigidBody;
  AudioComponent *     _audio;
  GraphicsComponent *  _graphics;
  
  vec3 _localPosition;
  quat _localOrientation;
  mutable vec3 _worldPosition;
  mutable quat _worldOrientation;
  vec3 _velocity;
  
  mutable bool _transformNeedsUpdating;
  mutable bool _enabled;
  
  void _updateTransform() const;
  
};


//
// MARK: - CoreOptions
//

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
  const ivec2 & mousePosition() const;
  const ivec2 & mouseMovement() const;
  
  int sampleRate() const;
  double maxVolume() const;
  
  const mat4 & viewMatrix() const;
  const mat4 & projectionMatrix() const;

  int scale() const;
  const vec3 & backgroundColor() const;
  
  static constexpr float UNITS_PER_METER = 0.01f;
  static const vec3 WORLD_UP;
  static const vec3 WORLD_DOWN;
  static const vec3 WORLD_LEFT;
  static const vec3 WORLD_RIGHT;
  static const vec3 WORLD_FORWARD;
  static const vec3 WORLD_BACKWARD;
  
  // MARK: Member functions
  Core(int numberOfEntities = 10000);
  bool init(CoreOptions & options);
  bool update();
  void destroy();
  void resolveCollisions(Entity & collider) const;
  
  static bool AABBIntersect(box & a, box & b, box & intersection);
  
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
  Entity * createEntity(string id, string parentId = "root");
  
  void createEffectiveTimer(double duration, function<void()> block);
  void createAccumulativeTimer(double duration, function<void()> block);
  void addControl(string name, SDL_Keycode key);
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
  // MARK: Private
  typedef map<string, pair<SDL_Keycode, bool>> _KeyControls;
  struct _Timer
  {
    double endTime;
    function<void(void)> block;
  };
  enum _TimerType { _EFFECTIVE, _ACCUMULATIVE };
  
  Entity   _root;
  Entity * _camera;
  
  double _deltaTime;
  ivec2  _mousePosition;
  ivec2  _mouseMovement;
  
  int    _sampleRate;
  double _maxVolume;
  
  mat4 _viewMatrix;
  mat4 _projectionMatrix;
  
  int  _scale;
  vec3 _backgroundColor;
  
  SDL_Window * _window;
  SDL_GLContext _context;
  
  int _entityCount;
  int _maximumNumberOfEntities;
  vector<Entity> _entities;
  
  _KeyControls _keyControls;
  vector<pair<_Timer, _TimerType>> _timers;
  
  double _pauseDuration;
  bool _reset;
  bool _pause;
  
};
