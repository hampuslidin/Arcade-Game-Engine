//
//  Player.cpp
//  Game Engine
//

#include "Player.hpp"
#include "Board.hpp"
#include "HUD.hpp"


//
// MARK: - PlayerInputComponent
//

// MARK: Member functions

void PlayerInputComponent::init(Entity * entity)
{
  ControllerInputComponent::init(entity);
  
  auto did_clear_board = [this](Event) { _did_clear_board = true; };
  auto did_collide_with_enemy = [this, entity](Event event)
  {
    if (airborn())
    {
      auto controller = (Controller*)entity;
      controller->board_position(controller->previous_board_position());
      controller->order(controller->previous_order());
    }
  };

  auto physics = entity->physics();
  NotificationCenter::observe(did_clear_board, DidClearBoard);
  NotificationCenter::observe(did_collide_with_enemy,
                              DidCollideWithEnemy,
                              physics);
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

double PlayerInputComponent::animation_ending_delay()
{
  return 0.15;
}

vector<pair<int, int>> PlayerInputComponent::board_position_changes()
{
  return {
    {-1,  0},
    { 1,  0},
    {-1, -1},
    { 1,  1}
  };
}


//
// MARK: - PlayerAnimationComponent
//

// MARK: Member functions

vector<Vector2> PlayerAnimationComponent::end_points()
{
  return {
    { 16, -24},
    {-16,  24},
    {-16, -24},
    { 16,  24}
  };
}

double PlayerAnimationComponent::animation_speed() { return 0.3; }


//
// MARK: - PlayerPhysicsComponent
//

// MARK: Member functions

PlayerPhysicsComponent::PlayerPhysicsComponent()
  : ControllerPhysicsComponent()
{
  collision_bounds({7, 4, 2, 12});
}

void PlayerPhysicsComponent::init(Entity * entity)
{
  ControllerPhysicsComponent::init(entity);

  auto did_move_out_of_view = [entity](Event)
  {
    entity->core()->pause();
    entity->core()->reset(1.0);
  };
  
  NotificationCenter::observe(did_move_out_of_view, DidMoveOutOfView, this);
}

void PlayerPhysicsComponent::collision_with_block(Block * block)
{
  block->touch();
}

void PlayerPhysicsComponent::collision_with_entity(Entity * entity)
{
  string id = entity->id();
  if (id.compare(0, 5, "enemy") == 0)
  {
    NotificationCenter::notify(DidCollideWithEnemy, *this);
    entity->core()->pause();
    entity->core()->reset(1.0);
  }
}


//
// MARK: - Player
//

Player::Player(string id)
  : Controller(id, 11)
{
  addInput(new PlayerInputComponent());
  addAnimation(new PlayerAnimationComponent());
  addPhysics(new PlayerPhysicsComponent());
  addGraphics(new PlayerGraphicsComponent());
}

void Player::init(Core * core)
{
  Controller::init(core);
  
  _should_revert = false;
  
  auto should_revert = [this](Event) { _should_revert = true; };
  
  NotificationCenter::observe(should_revert, DidClearBoard);
  NotificationCenter::observe(should_revert,
                              DidMoveOutOfView,
                              physics());
  NotificationCenter::observe(should_revert, DidDie);
}

void Player::reset()
{
  Controller::reset();
  
  if (_should_revert)
  {
    _should_revert = false;
    board_position(previous_board_position());
    order(previous_order());
  }
  
  const Dimension2 view_dimensions = core()->view_dimensions();
  const int row = board_position().first;
  const int column = board_position().second;
  const double x_pos = view_dimensions.x/2 - 8   - 16*row + 32*column;
  const double y_pos = view_dimensions.y   - 200 + 24*row;
  moveTo(x_pos, y_pos);
}

string Player::prefix_standing()                { return "qbert_standing"; }
string Player::prefix_jumping()                 { return "qbert_jumping";  }
int Player::direction_mask()                    { return 0b1111;           }
pair<int, int> Player::default_board_position() { return {0, 0};           }
int Player::default_order()                     { return 15;               }
ControllerDirection Player::default_direction() { return DOWN;             }
