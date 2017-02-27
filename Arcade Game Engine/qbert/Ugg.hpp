//
//  Ugg.hpp
//  Game Engine
//

#pragma once

#include "core.hpp"
#include "Controller.hpp"


//
// MARK: - UggInputComponent
//

class UggInputComponent
  : public ControllerInputComponent
{
protected:
  ControllerDirection update_direction(Core & core);
  double animation_ending_delay();
  vector<pair<int, int>> board_position_changes();
public:
};


//
// MARK: - UggAnimationComponent
//

class UggAnimationComponent
  : public ControllerAnimationComponent
{
protected:
  vector<Vector2> end_points();
  double animation_speed();
public:
};


//
// MARK: - UggPhysicsComponent
//

class UggPhysicsComponent
  : public ControllerPhysicsComponent
{
public:
  UggPhysicsComponent();
  void init(Entity * entity);
};


//
// MARK: - UggGraphicsComponent
//

typedef ControllerGraphicsComponent UggGraphicsComponent;


//
// MARK: - Ugg
//

class Ugg
  : public Controller
{
protected:
  int direction_mask();
  pair<int, int> default_board_position();
  int default_order();
public:
  ControllerDirection default_direction();
  
  Ugg();
  void reset();
  string prefix_standing();
  string prefix_jumping();
};
