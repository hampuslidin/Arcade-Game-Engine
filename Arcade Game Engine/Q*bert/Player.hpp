//
//  Player.hpp
//  Game Engine
//

#pragma once

#include "core.hpp"
#include "Controller.hpp"


//
// MARK: - PlayerInputComponent
//

class PlayerInputComponent
  : public ControllerInputComponent
{
  bool _did_clear_board;
protected:
  ControllerDirection update_direction(Core & core);
  double animation_ending_delay();
public:
  void init(Entity * entity);
  void reset();
};


//
// MARK: - PlayerAnimationComponent
//

class PlayerAnimationComponent
  : public ControllerAnimationComponent
{
protected:
  double animation_speed();
  Vector2 jump_up_end_point();
  Vector2 jump_down_end_point();
  Vector2 jump_left_end_point();
  Vector2 jump_right_end_point();
};


//
// MARK: - PlayerPhysicsComponent
//

class PlayerPhysicsComponent
  : public ControllerPhysicsComponent
{
  bool _has_jumped_once;
protected:
  bool should_break_for_collision(Entity * collided_entity);
  bool should_update();
public:
  PlayerPhysicsComponent();
  void init(Entity * entity);
  void reset();
};


//
// MARK: - PlayerGraphicsComponent
//

class PlayerGraphicsComponent
  : public ControllerGraphicsComponent
{
  string default_sprite_id();
};


//
// MARK: - Player
//

class Player
  : public Controller
{
protected:
  int direction_mask();
public:
  Player(string id);
  void reset();
  string prefix_standing();
  string prefix_jumping();
};
