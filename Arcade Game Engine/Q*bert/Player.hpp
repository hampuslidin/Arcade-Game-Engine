//
//  Player.hpp
//  Game Engine
//

#include "core.hpp"

// Events
const Event DidJump("DidJump");

/**
 *  Defines directions up, down, left, and right.
 */
enum PlayerDirection
{
  UP, DOWN, LEFT, RIGHT
};

/**
 *  Defines the player input mechanics.
 */
class PlayerInputComponent
  : public InputComponent
  , public Notifier
  , public Observer
{
public:
  prop_r<PlayerInputComponent, bool> jumping;
  
  void init(Entity * owner);
  void update(World & world);
  void onNotify(Entity & entity, Event event);
};

/**
 *  Defines the player animations.
 */
class PlayerAnimationComponent : public AnimationComponent, public Observer
{
public:
  void init(Entity * owner);
  void onNotify(Entity & entity, Event event);
};

/**
 *  Defines the player physics.
 */
class PlayerPhysicsComponent : public PhysicsComponent
{
public:
  PlayerPhysicsComponent();
};

/**
 *  Defines the player graphics.
 */
class PlayerGraphicsComponent : public GraphicsComponent, public Observer
{
public:
  void init(Entity * owner);
  void update(World & world);
  void onNotify(Entity & entity, Event event);
};

/**
 *  Defines a player.
 */
class Player : public Entity
{
public:
  Player();
  void init(World * owner);
};
