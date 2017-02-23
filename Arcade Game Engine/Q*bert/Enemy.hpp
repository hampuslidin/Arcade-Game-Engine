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
};


//
// MARK: - EnemyAnimationComponent
//

class EnemyAnimationComponent
  : public ControllerAnimationComponent
{
  double _speed;
  Vector2 _jump_up_end_point;
  Vector2 _jump_down_end_point;
  Vector2 _jump_left_end_point;
  Vector2 _jump_right_end_point;
protected:
  double animation_speed();
  Vector2 jump_up_end_point();
  Vector2 jump_down_end_point();
  Vector2 jump_left_end_point();
  Vector2 jump_right_end_point();
public:
  EnemyAnimationComponent(double speed,
                          Vector2 jump_up_end_point,
                          Vector2 jump_down_end_point,
                          Vector2 jump_left_end_point,
                          Vector2 jump_right_end_point);
};


//
// MARK: - EnemyPhysicsComponent
//

class EnemyPhysicsComponent
  : public ControllerPhysicsComponent
{
protected:
  bool should_break_for_collision(Entity * collided_entity);
  bool should_update();
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
protected:
  int direction_mask();
public:
  prop_r<Enemy, ControllerDirection> default_direction;
  
  Enemy(string id,
        ControllerDirection default_direction,
        double speed,
        Vector2 gravity,
        Vector2 jump_up_end_point,
        Vector2 jump_down_end_point,
        Vector2 jump_left_end_point,
        Vector2 jump_right_end_point);
  void reset();
  string prefix_standing();
  string prefix_jumping();
};
