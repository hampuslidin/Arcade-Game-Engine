//
//  Enemy.hpp
//  Game Engine
//

#pragma once

#include "core.hpp"
#include "Controller.hpp"


//
// MARK: - EnemyInputComponent
//

class EnemyInputComponent
  : public ControllerInputComponent
{
protected:
  ControllerDirection update_direction(Core & core);
  double animation_ending_delay();
public:
  EnemyInputComponent(pair<int, int> board_poistion_changes[4]);
};


//
// MARK: - EnemyAnimationComponent
//

class EnemyAnimationComponent
  : public ControllerAnimationComponent
{
  double _speed;
protected:
  double animation_speed();
public:
  EnemyAnimationComponent(double speed, Vector2 end_points[4]);
};


//
// MARK: - EnemyPhysicsComponent
//

class EnemyPhysicsComponent
  : public ControllerPhysicsComponent
{
protected:
  bool should_break_for_collision(Entity * collided_entity);
public:
  EnemyPhysicsComponent(Vector2 gravity);
};

//
// MARK: - EnemyGraphicsComponent
//

class EnemyGraphicsComponent
  : public ControllerGraphicsComponent
{
  string default_sprite_id();
};


//
// MARK: - Enemy
//

class Enemy
  : public Controller
{
  int _default_order;
  pair<int, int> _default_board_position;
protected:
  int direction_mask();
  int default_order();
  pair<int, int> default_board_position();
public:
  prop_r<Enemy, ControllerDirection> default_direction;
  
  Enemy(string id,
        int order,
        ControllerDirection default_direction,
        pair<int, int> default_board_position,
        double speed,
        Vector2 gravity,
        Vector2 end_points[4],
        pair<int, int> board_position_changes[4]);
  void reset();
  string prefix_standing();
  string prefix_jumping();
};
