//
//  Enemy.cpp
//  Game Engine
//

#include "Enemy.hpp"


//
// MARK: - EnemyInputComponent
//

// MARK: Member functions

ControllerDirection EnemyInputComponent::update_direction(Core & core)
{
  return arc4random_uniform(2)*2;
}

double EnemyInputComponent::animation_ending_delay() { return 0.2; };

//
// MARK: - EnemyAnimationComponent
//

// MARK: Member functions

double EnemyAnimationComponent::animation_speed() { return _speed; };
Vector2 EnemyAnimationComponent::jump_up_end_point()
{
  return _jump_up_end_point;
};
Vector2 EnemyAnimationComponent::jump_down_end_point()
{
  return _jump_down_end_point;
};
Vector2 EnemyAnimationComponent::jump_left_end_point()
{
  return _jump_left_end_point;
};
Vector2 EnemyAnimationComponent::jump_right_end_point()
{
  return _jump_right_end_point;
};

EnemyAnimationComponent::EnemyAnimationComponent(double speed,
                                                 Vector2 jump_up_end_point,
                                                 Vector2 jump_down_end_point,
                                                 Vector2 jump_left_end_point,
                                                 Vector2 jump_right_end_point)
  : ControllerAnimationComponent()
{
  _speed = speed;
  _jump_up_end_point = jump_up_end_point;
  _jump_down_end_point = jump_down_end_point;
  _jump_left_end_point = jump_left_end_point;
  _jump_right_end_point = jump_right_end_point;
}

//
// MARK: - EnemyPhysicsComponent
//

// MARK: Member functions

EnemyPhysicsComponent::EnemyPhysicsComponent(Vector2 gravity)
  : ControllerPhysicsComponent()
{
  collision_bounds({2, 0, 2, 2});
  this->gravity(gravity);
}

bool EnemyPhysicsComponent::should_break_for_collision(Entity * collided_entity)
{
  return true;
}

bool EnemyPhysicsComponent::should_update()
{
  return false;
}

//
// MARK: - EnemyGraphicsComponent
//

string EnemyGraphicsComponent::default_sprite_id()
{
  string direction_string = to_string(((Enemy*)entity())->default_direction());
  return id() + "_jumping_" + direction_string;
}


//
// MARK: - Enemy
//

Enemy::Enemy(string id,
             ControllerDirection default_direction,
             double speed,
             Vector2 gravity,
             Vector2 jump_up_end_point,
             Vector2 jump_down_end_point,
             Vector2 jump_left_end_point,
             Vector2 jump_right_end_point)
  : Controller(id,
               new EnemyInputComponent(),
               new EnemyAnimationComponent(speed,
                                           jump_up_end_point,
                                           jump_down_end_point,
                                           jump_left_end_point,
                                           jump_right_end_point),
               new EnemyPhysicsComponent(gravity),
               new EnemyGraphicsComponent())
{
  this->default_direction(default_direction);
}

void Enemy::reset()
{
  Controller::reset();
  
  const Dimension2 view_dimensions = core()->view_dimensions();
  moveTo(view_dimensions.x/2 + 70, view_dimensions.y-30);
}

string Enemy::prefix_standing() { return id() + "_jumping"; }
string Enemy::prefix_jumping()  { return id() + "_jumping"; }
int Enemy::direction_mask()     { return 0b1010;            }
