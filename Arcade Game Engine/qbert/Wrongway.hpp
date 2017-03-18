//
//  Wrongway.hpp
//  Game Engine
//

#pragma once

#include "core.hpp"
#include "Character.hpp"


//
// MARK: - WrongwayInputComponent
//

class WrongwayInputComponent
  : public CharacterInputComponent
{
protected:
  CharacterDirection update_direction(Core & core);
  double animation_ending_delay();
  vector<pair<int, int>> board_position_changes();
public:
};


//
// MARK: - WrongwayAnimationComponent
//

class WrongwayAnimationComponent
  : public CharacterAnimationComponent
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
  : public CharacterPhysicsComponent
{
public:
  WrongwayPhysicsComponent();
  void init(Entity * entity);
};


//
// MARK: - WrongwayGraphicsComponent
//

typedef CharacterGraphicsComponent WrongwayGraphicsComponent;


//
// MARK: - Wrongway
//

class Wrongway
  : public Character
{
protected:
  int direction_mask();
  pair<int, int> default_board_position();
  int default_order();
public:
  CharacterDirection default_direction();
  
  Wrongway();
  void reset();
  string prefix_standing();
  string prefix_jumping();
};
