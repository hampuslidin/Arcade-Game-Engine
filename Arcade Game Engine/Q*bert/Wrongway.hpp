//
//  Wrongway.hpp
//  Game Engine
//

#pragma once

#include "core.hpp"
#include "Controller.hpp"


//
// MARK: - WrongwayInputComponent
//

class WrongwayInputComponent
  : public ControllerInputComponent
{
protected:
  ControllerDirection update_direction(Core & core);
  double animation_ending_delay();
  vector<pair<int, int>> board_position_changes();
public:
};


//
// MARK: - WrongwayAnimationComponent
//

class WrongwayAnimationComponent
  : public ControllerAnimationComponent
{
protected:
  vector<Vector2> end_points();
  double animation_speed();
public:
};


//
// MARK: - WrongwayPhysicsComponent
//

class WrongwayPhysicsComponent
  : public ControllerPhysicsComponent
{
public:
  WrongwayPhysicsComponent();
  void init(Entity * entity);
};


//
// MARK: - WrongwayGraphicsComponent
//

typedef ControllerGraphicsComponent WrongwayGraphicsComponent;


//
// MARK: - Wrongway
//

class Wrongway
  : public Controller
{
protected:
  int direction_mask();
  pair<int, int> default_board_position();
  int default_order();
public:
  ControllerDirection default_direction();
  
  Wrongway();
  void reset();
  string prefix_standing();
  string prefix_jumping();
};
