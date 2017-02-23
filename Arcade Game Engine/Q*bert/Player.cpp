//
//  Player.cpp
//  Game Engine
//

#include "Player.hpp"
#include "Board.hpp"


//
// MARK: - PlayerInputComponent
//

// MARK: Member functions

void PlayerInputComponent::init(Entity * entity)
{
  ControllerInputComponent::init(entity);
  
  auto did_clear_board = [this](Event) { _did_clear_board = true; };

  NotificationCenter::observe(did_clear_board, DidClearBoard);
}

void PlayerInputComponent::reset()
{
  ControllerInputComponent::reset();
  
  _did_clear_board = false;
}

ControllerDirection PlayerInputComponent::update_direction(Core & core)
{
  if (!_did_clear_board)
  {
    Core::KeyStatus keys;
    core.keyStatus(keys);
    
    if (keys.up)
      return UP;
    if (keys.down)
      return DOWN;
    if (keys.left)
      return LEFT;
    if (keys.right)
      return RIGHT;
  }
  return NONE;
}

double PlayerInputComponent::animation_ending_delay() { return 0.15; }


//
// MARK: - PlayerAnimationComponent
//

// MARK: Member functions

double PlayerAnimationComponent::animation_speed()       { return 0.3; }
Vector2 PlayerAnimationComponent::jump_up_end_point()    { return { 16, -24}; };
Vector2 PlayerAnimationComponent::jump_down_end_point()  { return {-16,  24}; };
Vector2 PlayerAnimationComponent::jump_left_end_point()  { return {-16, -24}; };
Vector2 PlayerAnimationComponent::jump_right_end_point() { return { 16,  24}; };


//
// MARK: - PlayerPhysicsComponent
//

// MARK: Member functions

PlayerPhysicsComponent::PlayerPhysicsComponent()
  : ControllerPhysicsComponent()
{
  collision_bounds({7, 14, 2, 2});
}

void PlayerPhysicsComponent::init(Entity * entity)
{
  ControllerPhysicsComponent::init(entity);

  auto did_jump = [this](Event) { _has_jumped_once = true; };
  auto did_move_out_of_view = [entity](Event)
  {
    entity->order(-1);
    entity->core()->createTimer(1, [entity]()
    {
      entity->core()->reset();
    });
  };
  
  auto input = entity->input();
  NotificationCenter::observe(did_jump, DidJump, input);
  NotificationCenter::observe(did_move_out_of_view, DidMoveOutOfView, this);
}

void PlayerPhysicsComponent::reset()
{
  ControllerPhysicsComponent::reset();
  
  _has_jumped_once = false;
}

bool PlayerPhysicsComponent::should_break_for_collision(Entity * collided_entity)
{
  string id_prefix = collided_entity->id().substr(0, 5);
  if (collided_entity->id().compare(0, 5, "block") == 0)
  {
    ((Block*)collided_entity)->touch();
    return true;
  }
  return false;
}

bool PlayerPhysicsComponent::should_update()
{
  return _has_jumped_once;
}


//
// MARK: - PlayerGraphicsComponent
//

string PlayerGraphicsComponent::default_sprite_id()
{
  return "qbert_standing_1";
}


//
// MARK: - Player
//

Player::Player(string id)
  : Controller(id,
               new PlayerInputComponent(),
               new PlayerAnimationComponent(),
               new PlayerPhysicsComponent(),
               new PlayerGraphicsComponent())
{}

void Player::reset()
{
  Controller::reset();
  
  const Dimension2 view_dimensions = core()->view_dimensions();
  moveTo(view_dimensions.x/2-8, view_dimensions.y-176-16-8);
}

string Player::prefix_standing() { return "qbert_standing"; }
string Player::prefix_jumping()  { return "qbert_jumping";  }
int Player::direction_mask()     { return 0b1111;           }
