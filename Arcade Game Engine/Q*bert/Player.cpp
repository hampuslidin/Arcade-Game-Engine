//
//  Player.cpp
//  Game Engine
//

#include "Player.hpp"
#include "Board.hpp"
#include <fstream>


//
// MARK: - PlayerInputComponent
//

// MARK: Member functions

void PlayerInputComponent::init(Entity * entity)
{
  ControllerInputComponent::init(entity);
  
  auto did_clear_board = [this](Event, vector<GameObject*> *)
  {
    _did_clear_board = true;
  };

  NotificationCenter::main().observe(DidClearBoard, did_clear_board);
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
    
    if (keys.up)    return UP;
    if (keys.down)  return DOWN;
    if (keys.left)  return LEFT;
    if (keys.right) return RIGHT;
  }
  return NONE;
}


//
// MARK: - PlayerAnimationComponent
//

// MARK: Member functions

void PlayerAnimationComponent::init(Entity * entity)
{
  ControllerAnimationComponent::init(entity);
  
  addSegment("jump_up",    {0,     0}, {16,  -86.848});
  addSegment("jump_up",    {16,  -24}, {16,   38.848});
  
  addSegment("jump_down",  {0,     0}, {-16, -38.848});
  addSegment("jump_down",  {-16,  24}, {-16,  86.848});
  
  addSegment("jump_left",  {0,     0}, {-16, -86.848});
  addSegment("jump_left",  {-16, -24}, {-16,  38.848});
  
  addSegment("jump_right", {0,     0}, {16,  -38.848});
  addSegment("jump_right", {16,   24}, {16,   86.848});
}


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

  auto did_jump = [this](Event, vector<GameObject*> *)
  {
    _has_jumped_once = true;
  };
  auto did_move_out_of_view = [entity](Event, vector<GameObject*> *)
  {
    entity->order(-1);
    entity->core()->createTimer(1, [entity]() { entity->core()->reset(); });
  };
  
  NotificationCenter::main().observe(DidJump,          did_jump);
  NotificationCenter::main().observe(DidMoveOutOfView, did_move_out_of_view);
}

void PlayerPhysicsComponent::reset()
{
  ControllerPhysicsComponent::reset();
  
  _has_jumped_once = false;
}

bool PlayerPhysicsComponent::collision_event(Entity * collided_entity)
{
  string id_prefix = collided_entity->id().substr(0, 5);
  if (collided_entity->id().compare(0, 5, "block") == 0)
  {
    ((Block*)collided_entity)->toggle_state();
    return true;
  }
  return false;
}

bool PlayerPhysicsComponent::should_update()
{
  return _has_jumped_once;
}


//
// MARK: - Player
//

Player::Player(string id)
  : Controller(id,
               new PlayerInputComponent(),
               new PlayerAnimationComponent(),
               new PlayerPhysicsComponent(),
               new ControllerGraphicsComponent())
{}

void Player::reset()
{
  Controller::reset();
  
  const Dimension2 view_dimensions = core()->view_dimensions();
  moveTo(view_dimensions.x/2-8, view_dimensions.y-176-16-8);
}

string Player::prefix_standing()
{
  return "qbert_standing";
}

string Player::prefix_jumping()
{
  return "qbert_jumping";
}
