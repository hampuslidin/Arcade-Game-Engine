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

EnemyInputComponent::
  EnemyInputComponent(pair<int, int> board_position_changes[])
  : ControllerInputComponent(board_position_changes)
{}

//
// MARK: - EnemyAnimationComponent
//

// MARK: Member functions

double EnemyAnimationComponent::animation_speed() { return _speed; };

EnemyAnimationComponent::EnemyAnimationComponent(double speed,
                                                 Vector2 end_points[])
  : ControllerAnimationComponent(end_points)
{
  _speed = speed;
}

//
// MARK: - EnemyPhysicsComponent
//

// MARK: Member functions

EnemyPhysicsComponent::EnemyPhysicsComponent(Vector2 gravity)
  : ControllerPhysicsComponent()
{
  this->gravity(gravity);
}

void EnemyPhysicsComponent::init(Entity * entity)
{
  ControllerPhysicsComponent::init(entity);
  
  auto did_move_out_of_view = [entity](Event)
  {
    entity->enabled(false);
    entity->reset();
  };
  
  NotificationCenter::observe(did_move_out_of_view, DidMoveOutOfView, this);
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
             int order,
             ControllerDirection default_direction,
             pair<int, int> default_board_position,
             double speed,
             Vector2 gravity,
             Vector2 end_points[],
             pair<int, int> board_position_changes[])
  : Controller(id, order)
{
  addInput(new EnemyInputComponent(board_position_changes));
  addAnimation(new EnemyAnimationComponent(speed, end_points));
  addPhysics(new EnemyPhysicsComponent(gravity));
  addGraphics(new EnemyGraphicsComponent());
  
  _default_board_position = default_board_position;
  _default_order = order;
  this->default_direction(default_direction);
}

void Enemy::reset()
{
  Controller::reset();
  
  enabled(false);
  order(default_order());
  board_position(default_board_position());
  
  core()->createEffectiveTimer(arc4random_uniform(3)+2, [this]
  {
    enabled(true);
  });
  
  const Dimension2 view_dimensions = core()->view_dimensions();
  moveTo(view_dimensions.x/2 + 102, view_dimensions.y-30);
}

string Enemy::prefix_standing() { return id() + "_jumping";  }
string Enemy::prefix_jumping()  { return id() + "_jumping";  }
int Enemy::direction_mask()     { return 0b0101;             }
int Enemy::default_order()      { return _default_order;     }
pair<int, int> Enemy::default_board_position()
{
  return _default_board_position;
}
