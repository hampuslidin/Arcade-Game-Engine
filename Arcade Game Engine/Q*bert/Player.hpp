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
  PlayerInputComponent(pair<int, int> board_position_changes[4]);
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
public:
  PlayerAnimationComponent(Vector2 end_points[4]);
};


//
// MARK: - PlayerPhysicsComponent
//

class PlayerPhysicsComponent
  : public ControllerPhysicsComponent
{
protected:
  void collision(Entity * collided_entity);
public:
  PlayerPhysicsComponent();
  void init(Entity * entity);
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
  int default_order();
  pair<int, int> default_board_position();
public:
  Player(string id);
  void reset();
  string prefix_standing();
  string prefix_jumping();
};
