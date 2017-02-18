//
//  Player.hpp
//  Game Engine
//

#pragma once

#include "core.hpp"

// Events
const Event DidJump("DidJump");
const Event DidFallOff("DidFallOff");

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
{
  bool _animating;
  bool _did_jump_off;
public:
  void init(Entity * entity);
  void reset();
  void update(Core & core);
};

/**
 *  Defines the player animations.
 */
class PlayerAnimationComponent
  : public AnimationComponent
{
public:
  void init(Entity * entity);
};

/**
 *  Defines the player physics.
 */
class PlayerPhysicsComponent
  : public PhysicsComponent
{
  bool _has_jumped_once;
  bool _animating;
  bool _did_fall_off;
public:
  PlayerPhysicsComponent();
  void init(Entity * entity);
  void reset();
  void update(Core & core);
};

/**
 *  Defines the player graphics.
 */
class PlayerGraphicsComponent
  : public GraphicsComponent
{
  int _current_direction;
  bool _jumping;
public:
  void init(Entity * entity);
  void reset();
};

/**
 *  Defines a player.
 */
class Player : public Entity
{
public:
  Player(string id);
  void init(Core * core);
  void reset();
};
