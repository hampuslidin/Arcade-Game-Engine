//
//  core.hpp
//  Game Engine
//

#pragma once

#include <map>
#include <vector>
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
class World;
class Entity;
class Component;
class InputComponent;
class PhysicsComponent;
class GraphicsComponent;



/**
 *  A container for collision data.
 */
struct Collision {
  Entity * collider, *collided_entity;
  float overlap_distance;
};


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
  void addObserver(Observer * observer, Event event);
  void addObserver(Observer * observer, vector<Event> event);
  void removeObserver(Observer * observer, Event event = nullptr);
protected:
  prop<map<Event, vector<Observer*>>> observers;
  
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
 *  Defines the whole game world and is responsible for reading user input
 *  and updating all entities.
 */
class World
{
public:
  prop_r<World,     SDL_Window*> window;
  prop_r<World,   SDL_Renderer*> renderer;
  prop_r<World, vector<Entity*>> entities;
  prop_r<World,           float> delta_time;
  prop_r<World,       Rectangle> bounds;
  prop_r<World,             int> scale;
  
  /**
   *  Defines the status of each input type.
   */
  struct KeyStatus
  {
    bool up, down, left, right, fire;
  };
  
  bool init(const char * title,
            Dimension2 dimensions,
            RGBAColor background_color = {0xAA, 0xAA, 0xAA, 0xFF},
            int scale = 1);
  void destroy();
  void addEntity(Entity * entity);
  void removeEntity(Entity * entity);
  bool update();
  bool resolveCollisions(Entity & collider, bool collision_response = true);
  void getKeyStatus(KeyStatus & keys);
  float getElapsedTime();
private:
  KeyStatus _keys;
  float _prev_time;
  bool _initialized;
};

/**
 *  Defines a class that represents a game entity that resides in a game world.
 */
class Entity
{
public:
  prop_r<Entity,             World*> world;
  prop_r<Entity,    InputComponent*> input;
  prop_r<Entity,  PhysicsComponent*> physics;
  prop_r<Entity, GraphicsComponent*> graphics;
  prop_r<Entity,            Vector2> pos;
  prop_r<Entity,            Vector2> vel;
  prop<bool> is_dynamic;
  
  Entity(InputComponent * input,
         PhysicsComponent * physics,
         GraphicsComponent * graphics);
  ~Entity();
  virtual void init(World * owner);
  void update(World & world);
  void moveTo(float x, float y);
  void moveBy(float dx, float dy);
  void changeVelocityTo(float vx, float vy);
  void changeVelocityBy(float dvs, float dvy);
};


/**
 *  An abstrct class for a generic component.
 */
class Component
{
protected:
  prop_r<Component, Entity*> entity;
public:
  virtual void init(Entity * owner);
  virtual void update(World & world) = 0;
};


/**
 *  An abstrct class that is responsible for defining the behavior of an Entity
 *  object, possibly depending on user input.
 */
class InputComponent : public Component
{
public:
  virtual ~InputComponent() {};
};


/**
 *  An abstract class that is responsible for updating the position and velocity
 *  of an Entity object.
 */
class PhysicsComponent : public Component
{
public:
  prop<Rectangle> collision_bounds;
  
  PhysicsComponent() {};
  PhysicsComponent(Rectangle collision_bounds);
  virtual ~PhysicsComponent() {};
};


/**
 *  An abstract class that is responsible for drawing an Entity object to a
 *  given SDL rendering context.
 */
class GraphicsComponent : public Component
{
protected:
  prop<vector<Sprite*>> sprites;
  prop<        Sprite*> current_sprite;
  prop<      Rectangle> bounds;
  
  void initSprites(SDL_Renderer & renderer, vector<const char *> files);
public:
  virtual ~GraphicsComponent();
  void offsetTo(int x, int y);
  void offsetBy(int dx, int dy);
  void resizeTo(int w, int h);
  void resizeBy(int dw, int dh);
};
