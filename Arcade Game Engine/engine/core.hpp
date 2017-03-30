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
class MeshComponent;

// MARK: Events

const Event DidStartAnimating("DidStartAnimating");
const Event DidStopAnimating("DidStopAnimating");
const Event DidCollide("DidCollide");
const Event DidMoveIntoView("DidMoveIntoView");
const Event DidMoveOutOfView("DidMoveOutOfView");
const Event DidUpdateTransform("DidUpdateTransform");


//
// MARK: - Sprite
//

/**
 *  Defines a sprite and methods for drawing it to a SDL rendering context.
 */
class Sprite
{
  SDL_Renderer * _renderer;
  SDL_Texture * _texture;
public:
  Sprite(SDL_Renderer * renderer, SDL_Texture * texture);
  static Sprite * createSprite(SDL_Renderer * renderer, const char * filename);
  void destroy();
  void draw(int x, int y, int w, int h, int scale = 1);
};


//
// MARK: - SpriteCollection
//

/**
 *  Defines a collection of sprites.
 */
class SpriteCollection
{
  SDL_Renderer * _renderer;
  map<string, Sprite*> _sprites;
  
  SpriteCollection() {};
public:
  SpriteCollection(SpriteCollection const &) = delete;
  static SpriteCollection & main();
  void init(SDL_Renderer * renderer);
  Sprite * create(string id, const char * filename);
  void destroy(string id);
  void destroyAll();
  Sprite * retrieve(string id);
  void draw(string id, int x, int y, int w, int h, int scale = 1);
  
  void operator=(SpriteCollection const &) = delete;
};


//
// MARK: - NotificationCenter
//

typedef size_t ObserverID;

class NotificationCenter
{
  map<Event, vector<pair<function<void(Event)>, GameObject*>>> _blocks;
  
  NotificationCenter() {};
  static NotificationCenter & _instance();
public:
  static void notify(Event event, GameObject & sender);
  static ObserverID observe(function<void(Event)> block,
                            Event event,
                            GameObject * sender);
  static void unobserve(ObserverID id,
                        Event event,
                        GameObject * sender);
};


//
// MARK: - Synthesizer
//

class Synthesizer
{
  
public:
  enum WaveType
  {
    SMOOTH,
    TRIANGLE,
    SAWTOOTH,
    SQUARE
  };
  enum PitchGlideType
  {
    LINEAR,
    EXPONENTIAL,
    LOGARITHMIC,
    INV_LOGARITHMIC
  };
  
  prop<int> bit_rate;
  prop<int> sample_rate;
  
  Synthesizer(int bit_rate = 8, int sample_rate = 44100);
  void load(const char * filename);
  void select(string id);
  bool generate(int16_t * stream,
                int length,
                int & frame,
                double max_volume,
                double duration,
                double fade_in,
                double fade_out);
  
private:
  class _Operator
  {
    
  public:
    double frequency;
    double modulation_index;
    WaveType wave_type;
    double threshold_low;
    double threshold_high;
    maybe<double> pitch_glide;
    PitchGlideType pitch_glide_type;
    vector<_Operator*> modulators;
    
    _Operator(double frequency = 440,
              double modulation_index = 1.0,
              WaveType wave_type = SMOOTH,
              double threshold_low = -1.0,
              double threshold_high = 1.0,
              maybe<double> pitch_glide = maybe<double>::nothing(),
              PitchGlideType pitch_glide_type = EXPONENTIAL);
    void addModulator(_Operator * modulator);
    double calculateSample(double time, double duration);
    
  private:
    double _calculatePhase(double time, double duration);
    
  };
  struct _Algorithm
  {
    vector<_Operator> operators;
    int num_carriers;
  };
  
  map<string, _Algorithm> _algorithms;
  _Algorithm * _current_algorithm;
  
};


//
// MARK: - GameObject
//

class GameObject {
  
public:
  prop<string> id;
  
};


//
// MARK: - Entity
//

/**
 *  Defines a class that represents a game entity that resides in a game world.
 */
class Entity
  : public GameObject
{
  
public:
  prop_r<Entity, Core*>           core;
  prop_r<Entity, Entity*>         parent;
  prop_r<Entity, vector<Entity*>> children;
  
  prop<InputComponent*>           pInput;
  prop<AnimationComponent*>       pAnimation;
  prop<ColliderComponent*>        pCollider;
  prop<RigidBodyComponent*>       pRigidBody;
  prop<AudioComponent*>           pAudio;
  prop<MeshComponent*>            pMesh;
  
  prop_r<Entity, vec3>            localPosition;
  prop_r<Entity, quat>            localOrientation;
  
  prop<vec3>                      pVelocity;
  prop<bool>                      pEnabled;
  
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
  
  Dimension2 dimensions();
  
  /**
   *  Adds a child.
   *
   *  @param  child   A reference to an existing Entity.
   */
  void addChild(Entity * child);
  Entity * findChild(string id);
  void removeChild(string id);
  
  mat4 localTranslation();
  mat4 localRotation();
  vec3 localRight();
  vec3 localUp();
  vec3 localForward();
  
  vec3 worldPosition();
  quat worldOrientation();
  mat4 worldTranslation();
  mat4 worldRotation();
  
  void translate(float dx, float dy, float dz);
  void rotate(float angle, vec3 axis);
  
  void setPosition(float x, float y, float z);
  void setPositionX(float x);
  void setPositionY(float y);
  void setPositionZ(float z);
  
  void update(uint8_t component_mask);
  
private:
  bool _transformNeedsUpdating;
  vec3 _worldPosition;
  quat _worldOrientation;
  
  void _updateTransform();
  
};


/**
 *  An abstrct class for a generic component.
 */
class Component
  : public GameObject
{
protected:
  prop_r<Component, Entity*> entity;
  
  virtual string trait() = 0;
public:
  string id();
  
  virtual ~Component() {};
  virtual void init(Entity * entity);
  virtual void reset() {};
  virtual void update(Core & core) = 0;
};


/**
 *  InputConponent is responsible for defining the behavior of an Entity.
 */
class InputComponent
  : public Component
{
  string trait();
};


/**
 *  AnimationComponent is responsible for moving an Entity according to a path,
 *  either in local space or in world space.
 */
class AnimationComponent
: public Component
{
  
public:
  typedef vector<pair<vec3, vec3>> CubicHermiteCurve;
  typedef pair<pair<vec3, vec3>, pair<vec3, vec3>> CubicHermiteSpline;
  
  prop_r<AnimationComponent, bool> animating;
  prop_r<AnimationComponent, vec3> endVelocity;
  
  virtual void reset();
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
  
  virtual void update(Core & core);
  
private:
  string trait();
  
  map<string, CubicHermiteCurve> _curves;
  CubicHermiteCurve _currentCurve;
  vec3 _startPosition;
  double _startTime;
  double _duration;
  bool _updateVelocity;
};


/**
 *  ColliderComponent is responsible for setting the bounds for collision 
 *  detection between two colliders.
 */
class ColliderComponent
  : public Component
{
  
public:
  void collide(ColliderComponent & obsticle);
  virtual void update(Core & core) {};
  
private:
  bool _didCollide;
  
};


/**
 *  AABBColliderComponent does collision detection for axis-aligned bounding 
 *  boxes.
 */
class AABBColliderComponent
  : public ColliderComponent
{
  
public:
  prop<vec3> pMin;
  prop<vec3> pMax;
  
private:
  string trait();
  
};


/**
 *  RigidBodyComponent is responsible for updating the position of an Entity
 *  object, w.r.t. the laws of physics.
 */
class RigidBodyComponent
  : public Component
{

public:
  prop<vec3> pGravity;
  prop<bool> pIsKinematic;
  
  RigidBodyComponent();
  virtual void init(Entity * entity);
  virtual void update(Core & core);
  
private:
  bool _shouldSimulate;
  
  string trait();
};


/**
 *  AudioComponent is responsible for generating and playing sounds.
 */
class AudioComponent
: public Component
{
  
public:
  friend Core;
  
  virtual void init(Entity * entity);
  virtual void update(Core & core) {};
  
protected:
  prop_r<AudioComponent, Synthesizer> synthesizer;
  
  void playSound(string id,
                 double duration,
                 double fade_in = 0.01,
                 double fade_out = 0.01);
  void audioStreamCallback(double max_volume, int16_t * stream, int length);
  
private:
  struct _Audio
  {
    string id;
    double duration;
    double fade_in;
    double fade_out;
    int frame;
  };
  vector<_Audio> _audio_playback;
  
  string trait();
  
};

/**
 *  MeshComponent is responsible for drawing an Entity to a SDL rendering
 *  context.
 */
class MeshComponent
  : public Component
{
  
public:
  prop_r<MeshComponent, Rectangle>    bounds;
  
  prop_r<MeshComponent, vector<vec3>> vertexPositions;
  prop_r<MeshComponent, vector<vec3>> vertexColors;
  prop_r<MeshComponent, vector<int>>  vertexIndices;
  
  virtual void init(Entity * entity);
  virtual void update(Core & core);
  void attachMesh(vector<vec3> & positions,
                  vector<vec3> & colors,
                  vector<int> & indices);
  void attachShader(string vertexShaderFilename, string fragmentShaderFilename);
  void offsetTo(int x, int y);
  void offsetBy(int dx, int dy);
  void resizeTo(int w, int h);
  void resizeBy(int dw, int dh);
  
private:
  string trait();
  
  GLuint _vertexArrayObject;
  GLuint _positionBuffer;
  GLuint _colorBuffer;
  GLuint _indexBuffer;
  
  GLuint _shaderProgram;
  string _vertexShaderFilename;
  string _fragmentShaderFilename;

  GLint _modelViewProjectionMatrixLocation;
  
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
  prop_r<Core, Entity>   root;
  prop_r<Core, Entity*>  camera;
  
  prop_r<Core, double>   deltaTime;
  prop_r<Core, ivec2>    mousePosition;
  prop_r<Core, ivec2>    mouseMovement;
  
  prop_r<Core, int>      sampleRate;
  prop_r<Core, double>   maxVolume;
  
  prop_r<Core, mat4>     projectionMatrix;

  prop<int>              pScale;
  prop<vec3>             pBackgroundColor;
  
  static constexpr float unitsPerMeter = 1;
  
  double elapsedTime();
  double effectiveElapsedTime();
  ivec2 viewDimensions();
  
  Core(int numberOfEntities = 10000);
  bool init(CoreOptions & options);
  bool update();
  void destroy();

  void resolveCollisions(Entity & collider);
  
  static bool AABBCollision(vec3 colliderMin,
                            vec3 colliderMax,
                            vec3 obsticleMin,
                            vec3 obsticleMax);
  
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
  maybe<bool> checkKey(string name);
  void reset(double after_duration = 0);
  void pause();
  void resume();
  
private:
  typedef map<string, pair<SDL_Keycode, bool>> _KeyControls;
  
  struct _Timer
  {
    double endTime;
    function<void(void)> block;
  };
  
  enum _TimerType { _EFFECTIVE, _ACCUMULATIVE };
  
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
