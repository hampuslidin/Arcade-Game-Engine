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

PlayerInputComponent::
  PlayerInputComponent(pair<int, int> board_position_changes[])
  : ControllerInputComponent(board_position_changes)
{}

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

double PlayerAnimationComponent::animation_speed() { return 0.3; }

PlayerAnimationComponent::PlayerAnimationComponent(Vector2 end_points[])
  : ControllerAnimationComponent(end_points)
{}


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

  auto did_move_out_of_view = [entity](Event)
  {
    entity->core()->createTimer(1, [entity]()
    {
      entity->core()->reset();
    });
  };
  
  NotificationCenter::observe(did_move_out_of_view, DidMoveOutOfView, this);
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
               11,
               new PlayerInputComponent((pair<int, int>[]){{-1,  0},
                                                           { 1,  0},
                                                           {-1, -1},
                                                           { 1,  1}}),
               new PlayerAnimationComponent((Vector2[]){{ 16, -24},
                                                        {-16,  24},
                                                        {-16, -24},
                                                        { 16,  24}}),
               new PlayerPhysicsComponent(),
               new PlayerGraphicsComponent())
{}

void Player::reset()
{
  Controller::reset();
  
  const Dimension2 view_dimensions = core()->view_dimensions();
  moveTo(view_dimensions.x/2-8, view_dimensions.y-176-16-8);
}

string Player::prefix_standing()                { return "qbert_standing"; }
string Player::prefix_jumping()                 { return "qbert_jumping";  }
int Player::direction_mask()                    { return 0b1111;           }
int Player::default_order()                     { return 15;               }
pair<int, int> Player::default_board_position() { return {0, 0};           }
