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
class Notifier;
class Observer;
class Core;
class Entity;
class Component;
class InputComponent;
class AnimationComponent;
class PhysicsComponent;
class GraphicsComponent;

// Events
const Event DidStartAnimating("DidStartAnimating");
const Event DidStartMovingInAnimation("DidStartMovingInAnimation");
const Event DidStopAnimating("DidStopAnimating");


/**
 *  Defines a sprite and methods for drawing it to a SDL rendering context.
 */
class Sprite
{
  SDL_Renderer * _renderer;
  SDL_Texture * _texture;
public:
  Sprite(SDL_Renderer * renderer, SDL_Texture * texture);
  void destroy();
  void draw(int x, int y, int w, int h, int scale = 1);
};


/**
 *  An abstract class for notifying Observer objects of an event.
 */
class Notifier
{
public:
  void addObserver(Observer * observer, const Event & event);
  void removeObserver(Observer * observer, Event * event = nullptr);
protected:
  prop<map<Event, vector<Observer *>>> observers;
  
  virtual ~Notifier() {};
  void notify(Entity & entity, Event event);
};


/**
 *  An abstract class for receiving notifications about events from Notifier
 *  objects.
 */
class Observer
{
public:
  virtual void onNotify(Entity & entity, Event event) = 0;
};


/**
 *  Defines the core engine and is responsible for reading user input
 *  and updating all entities.
 */
class Core
{
public:
  prop_r<Core,   SDL_Window*> window;
  prop_r<Core, SDL_Renderer*> renderer;
  prop_r<Core,       Entity*> root;
  prop_r<Core,        double> delta_time;
  prop_r<Core,    Dimension2> view_dimensions;
  prop<int> scale;
  
  /**
   *  Defines the status of each input type.
   */
  struct KeyStatus
  {
    bool up, down, left, right, fire;
  };
  
  Core();
  bool init(Entity * root,
            const char * title,
            Dimension2 dimensions,
            RGBAColor background_color = {0xAA, 0xAA, 0xAA, 0xFF});
  void destroy();
//  void addEntity(Entity * entity);
//  void removeEntity(Entity * entity);
  bool update();
  bool resolveCollisions(Entity & collider, bool collision_response = true);
  void getKeyStatus(KeyStatus & keys);
  double elapsedTime();
private:
  KeyStatus _keys;
  double _prev_time;
  bool _initialized;
};

/**
 *  Defines a class that represents a game entity that resides in a game world.
 */
class Entity
{
public:
  prop_r<Entity,                         Core*> core;
  prop_r<Entity,                       Entity*> parent;
  prop_r<Entity, vector<pair<string, Entity*>>> children;
  prop_r<Entity,               InputComponent*> input;
  prop_r<Entity,           AnimationComponent*> animation;
  prop_r<Entity,             PhysicsComponent*> physics;
  prop_r<Entity,            GraphicsComponent*> graphics;
  prop_r<Entity,                       Vector2> local_position;
  prop_r<Entity,                       Vector2> velocity;
  
  Entity(InputComponent * input,
         AnimationComponent * animation,
         PhysicsComponent * physics,
         GraphicsComponent * graphics);
  
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
   */
  void addChild(Entity * child, string id);
  
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
  void update();
};


/**
 *  An abstrct class for a generic component.
 */
class Component
{
protected:
  prop_r<Component, Entity*> entity;
public:
  virtual ~Component() {};
  virtual void init(Entity * entity);
  virtual void update(Core & core) = 0;
};


/**
 *  InputConponent is responsible for defining the behavior of an Entity.
 */
class InputComponent : public Component {};


/**
 *  AnimationComponent is responsible for moving an Entity according to a path,
 *  either in local space or in world space.
 */
class AnimationComponent : public Component, public Notifier
{
  struct _AnimationPath
  {
    vector<Vector2> movements;
    double          duration;
  };
  
  map<string, _AnimationPath> _animation_paths;
  _AnimationPath _animation;
  double _animation_start_time;
  int _current_movement_index;
  bool _calculate_velocity;
public:
  prop_r<AnimationComponent, bool> animating;
  
  AnimationComponent();
  
  /**
   *  Loads an animation by reading the file at the specified location.
   *  
   *  @return true on success, false when failing to read the file.
   */
  bool loadAnimationFromFile(string filename,
                             string animation_id,
                             double duration);
  
  /**
   *  Removes an animation with the given id.
   *
   *  @return true on success, false if animation with associated id does not
   *  exist.
   */
  bool removeAnimation(string id);
  
  /**
   *  Initiates an animation, which will get updated by the *update* member
   *  function.
   *
   *  @return 0 on success, 1 if animation with associated id does not exist.
   */
  bool performAnimation(string id, bool calculate_velocity = false);
  virtual void update(Core & core);
};


/**
 *  PhysicsComponent is responsible for updating the position of an Entity
 *  object, w.r.t. the laws of physics.
 */
class PhysicsComponent : public Component, public Observer
{
  bool _should_simulate;
public:
  prop<   Rectangle> collision_bounds;
  prop<        bool> dynamic;
  prop<        bool> simulate_with_animations;
  prop<      double> gravity;
  prop<unsigned int> pixels_per_meter;
  
  PhysicsComponent();
  virtual void init(Entity * entity);
  virtual void update(Core & core);
  void onNotify(Entity & entity, Event event);
};


/**
 *  GraphicsComponent is responsible for drawing an Entity to a SDL rendering
 *  context.
 */
class GraphicsComponent : public Component
{
protected:
  prop<vector<Sprite*>> sprites;
  prop<        Sprite*> current_sprite;
  prop<      Rectangle> bounds;
  
  void initSprites(SDL_Renderer & renderer,
                   vector<const char *> files,
                   int current_sprite_index = 0);
public:
  virtual ~GraphicsComponent();
  void offsetTo(int x, int y);
  void offsetBy(int dx, int dy);
  void resizeTo(int w, int h);
  void resizeBy(int dw, int dh);
  virtual void update(Core & core);
};
