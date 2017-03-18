//
//  Ugg.hpp
//  Game Engine
//

#pragma once

#include "core.hpp"
#include "Character.hpp"


//
// MARK: - UggInputComponent
//

class UggInputComponent
  : public CharacterInputComponent
{
protected:
  CharacterDirection update_direction(Core & core);
  double animation_ending_delay();
  vector<pair<int, int>> board_position_changes();
public:
};


//
// MARK: - UggAnimationComponent
//

class UggAnimationComponent
  : public CharacterAnimationComponent
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
  : public CharacterPhysicsComponent
{
public:
  UggPhysicsComponent();
  void init(Entity * entity);
};


//
// MARK: - UggGraphicsComponent
//

typedef CharacterGraphicsComponent UggGraphicsComponent;


//
// MARK: - Ugg
//

class Ugg
  : public Character
{
protected:
  int direction_mask();
  pair<int, int> default_board_position();
  int default_order();
public:
  CharacterDirection default_direction();
  
  Ugg();
  void reset();
  string prefix_standing();
  string prefix_jumping();
};
