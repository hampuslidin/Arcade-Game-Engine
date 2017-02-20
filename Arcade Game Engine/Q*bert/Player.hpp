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
public:
  void init(Entity * entity);
};


//
// MARK: - PlayerPhysicsComponent
//

class PlayerPhysicsComponent
  : public ControllerPhysicsComponent
{
  bool _has_jumped_once;
protected:
  bool collision_event(Entity * collided_entity);
  bool should_update();
public:
  PlayerPhysicsComponent();
  void init(Entity * entity);
  void reset();
};


//
// MARK: - Player
//

class Player
  : public Controller
{
public:
  Player(string id);
  void reset();
  string prefix_standing();
  string prefix_jumping();
};
