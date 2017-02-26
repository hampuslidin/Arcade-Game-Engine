//
//  core.hpp
//  Game Engine
//

#pragma once

#include <map>
#include <vector>
#include <string>
#include "types.hpp"

#ifdef __APPLE__
# include <SDL2/SDL.h>
# include <SDL2_image/SDL_image.h>
#elif defined(__WIN32__)
# include "SDL.h"
# include "SDL_image.h"
#endif

using namespace std;

class Sprite;
class SpriteCollection;
class NotificationCenter;
class Timer;
class Core;
class GameObject;
class Entity;
class Component;
class InputComponent;
class AnimationComponent;
class PhysicsComponent;
class GraphicsComponent;

// MARK: Events

const Event DidStartAnimating("DidStartAnimating");
const Event DidStopAnimating("DidStopAnimating");
const Event DidCollide("DidCollide");
const Event DidMoveIntoView("DidMoveIntoView");
const Event DidMoveOutOfView("DidMoveOutOfView");


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
                            GameObject * sender = nullptr);
  static void unobserve(ObserverID id,
                        Event event,
                        GameObject * sender = nullptr);
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
  /**
   *  Defines the status of each input type.
   */
  struct KeyStatus
  {
    bool up, down, left, right;
  };
private:
  struct _Timer
  {
    double end_time;
    function<void(void)> block;
  };
  
  KeyStatus _key_status;
  vector<_Timer> _timers;
  double _pause_duration;
  bool _reset;
  bool _pause;
public:
  prop_r<Core,      SDL_Window*> window;
  prop_r<Core,    SDL_Renderer*> renderer;
  prop_r<Core,          Entity*> root;
  prop_r<Core,           double> delta_time;
  prop_r<Core,       Dimension2> view_dimensions;
  prop<int> scale;
  
  Core();
  bool init(Entity * root,
            const char * title,
            Dimension2 dimensions,
            RGBAColor background_color = {0x00, 0x00, 0x00, 0xFF});
  void destroy();
  void reset();
  void createTimer(double duration, function<void(void)> block);
  bool update();
  void resolveCollisions(Entity & collider,
                         bool collision_response,
                         vector<Entity*> & result);
  void keyStatus(KeyStatus & keys);
  double elapsedTime();
  double effectiveElapsedTime();
};



//
// MARK: - GameObject
//

class GameObject {
public:
  virtual string id() = 0;
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
  string _id;
public:
  
  prop_r<Entity,               Core*> core;
  prop_r<Entity,             Entity*> parent;
  prop_r<Entity,     vector<Entity*>> children;
  prop_r<Entity,     InputComponent*> input;
  prop_r<Entity, AnimationComponent*> animation;
  prop_r<Entity,   PhysicsComponent*> physics;
  prop_r<Entity,  GraphicsComponent*> graphics;
  prop_r<Entity,             Vector2> local_position;
  prop_r<Entity,             Vector2> velocity;
  prop<int> order;
  
  string id();
    
  // MARK: Member functions
  
  Entity(string id, int order);
  void addInput(InputComponent * input);
  void addAnimation(AnimationComponent * animation);
  void addPhysics(PhysicsComponent * physics);
  void addGraphics(GraphicsComponent * graphics);
  
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
   *  Adds a child with a given identity string to the entity.
   *
   *  Children must be constructed using the new operator. The children will
   *  be deleted either by calling *removeChild*, or by calling *destroy* on
   *  the entity.
   *
   *  @param  child   The Entity to be added.
   *  @param  order   The order in which the entity will be placed. If -1 or a
   *                  number equal to or larger than the number of children is
   *                  passed, then the entity will be placed at the back. 
   */
  void addChild(Entity * child, int order = -1);
  
  Entity * findChild(string id);
  void removeChild(string id);
  void calculateWorldPosition(Vector2 & result);
  void moveTo(double x, double y);
  void moveHorizontallyTo(double x);
  void moveVerticallyTo(double y);
  void moveBy(double dx, double dy);
  void changeVelocityTo(double vx, double vy);
  void changeHorizontalVelocityTo(double vx);
  void changeVerticalVelocityTo(double vy);
  void changeVelocityBy(double dvs, double dvy);
  void update(uint8_t mask = 0b1111);
};

class EntityCompare {
public:
  bool operator()(Entity * l, Entity * r);
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
  typedef vector<pair<Vector2, Vector2>> CubicHermiteCurve;
  typedef pair<pair<Vector2, Vector2>, pair<Vector2, Vector2>>
    CubicHermiteSpline;
private:
  string trait();
  
  map<string, CubicHermiteCurve> _curves;
  CubicHermiteCurve _current_curve;
  Vector2 _start_position;
  double _start_time;
  double _duration;
  bool _update_velocity;
public:
  
  prop_r<AnimationComponent, bool> animating;
  
  virtual void reset();
  
  void addSegment(string id, Vector2 point, Vector2 velocity);
  void removeCurve(string id);
  
  /**
   *  Initiates an animation, which will get updated by the *update* member
   *  function.
   *
   *  @return 0 on success, 1 if animation with associated id does not exist.
   */
  void performAnimation(string id,
                        double duration,
                        bool update_velocity = false);

  virtual void update(Core & core);
};


/**
 *  PhysicsComponent is responsible for updating the position of an Entity
 *  object, w.r.t. the laws of physics.
 */
class PhysicsComponent
  : public Component
{
  bool _should_simulate;
  bool _out_of_view;
  bool _did_collide;
  
  string trait();
protected:
  prop_r<PhysicsComponent, vector<Entity*>> collided_entities;
public:
  static constexpr int pixels_per_meter = 120;
  prop<   Rectangle> collision_bounds;
  prop<     Vector2> gravity;
  prop<        bool> dynamic;
  prop<        bool> collision_detection;
  prop<        bool> collision_response;
  prop<        bool> simulate_with_animations;
  
  PhysicsComponent();
  virtual void init(Entity * entity);
  virtual void update(Core & core);
};


/**
 *  GraphicsComponent is responsible for drawing an Entity to a SDL rendering
 *  context.
 */
class GraphicsComponent : public Component
{
  string trait();
protected:
  prop<  Sprite*> current_sprite;
  prop<Rectangle> bounds;
public:
  void offsetTo(int x, int y);
  void offsetBy(int dx, int dy);
  void resizeTo(int w, int h);
  void resizeBy(int dw, int dh);
  virtual void update(Core & core);
};
