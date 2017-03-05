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
  vector<pair<int, int>> board_position_changes();
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
  vector<Vector2> end_points();
  double animation_speed();
};


//
// MARK: - PlayerPhysicsComponent
//

class PlayerPhysicsComponent
  : public ControllerPhysicsComponent
{
protected:
  void collision_with_block(Block * block);
  void collision_with_entity(Entity * entity);
public:
  PlayerPhysicsComponent();
  void init(Entity * entity);
};


//
// MARK: - PlayerAudioComponent
//

class PlayerAudioComponent
  : public AudioComponent
{
public:
  void init(Entity * entity);
};


//
// MARK: - PlayerGraphicsComponent
//

typedef ControllerGraphicsComponent PlayerGraphicsComponent;


//
// MARK: - Player
//

class Player
  : public Controller
{
  bool _should_revert;
protected:
  int direction_mask();
  pair<int, int> default_board_position();
  int default_order();
  ControllerDirection default_direction();
public:
  Player(string id);
  void init(Core * core);
  void reset();
  string prefix_standing();
  string prefix_jumping();
};
