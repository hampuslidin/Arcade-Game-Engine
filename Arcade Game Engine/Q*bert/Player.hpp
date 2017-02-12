//
//  Player.hpp
//  Game Engine
//

#include "core.hpp"

/**
 *  Defines the player input mechanics.
 */
class PlayerInputComponent : public InputComponent, public Notifier {
public:
  prop_r<PlayerInputComponent, bool> jumping;
  
  static constexpr Event DidJumpUp    = "DidJumpUp";
  static constexpr Event DidJumpDown  = "DidJumpDown";
  static constexpr Event DidJumpLeft  = "DidJumpLeft";
  static constexpr Event DidJumpRight = "DidJumpRight";
  static constexpr Event DidJump      = "DidJump";
  
  PlayerInputComponent();
  void update(World & world);
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
  prop<Vector2> speed;
  
  Player();
  void init(World * owner);
};
