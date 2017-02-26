//
//  Controller.cpp
//  Game Engine
//

#include "Controller.hpp"
#include "Board.hpp"
#include <fstream>


//
// MARK: - ControllerInputComponent
//

// MARK: Member functions

ControllerInputComponent::
  ControllerInputComponent(pair<int, int> board_position_changes[4])
  : InputComponent()
{
  _board_position_changes[UP]    = board_position_changes[UP];
  _board_position_changes[DOWN]  = board_position_changes[DOWN];
  _board_position_changes[LEFT]  = board_position_changes[LEFT];
  _board_position_changes[RIGHT] = board_position_changes[RIGHT];
}

void ControllerInputComponent::init(Entity * entity)
{
  InputComponent::init(entity);
  
  auto did_start_animating = [this](Event)
  {
    _airborn = true;
    _animating = true;
  };
  auto did_stop_animating  = [this, entity](Event)
  {
    entity->core()->createEffectiveTimer(animation_ending_delay(), [this]
    {
      _animating = false;
    });
  };
  auto did_collide = [this](Event) { _airborn = false; };
  
  auto animation = entity->animation();
  auto physics   = entity->physics();
  NotificationCenter::observe(did_start_animating,
                              DidStartAnimating,
                              animation);
  NotificationCenter::observe(did_stop_animating,
                              DidStopAnimating,
                              animation);
  NotificationCenter::observe(did_collide,
                              DidCollide,
                              physics);
}

void ControllerInputComponent::reset()
{
  InputComponent::reset();
  
  _animating = false;
  _airborn = false;
}

void ControllerInputComponent::update(Core & core)
{
  if (!_animating && !_airborn)
  {
    auto controller = (Controller*)entity();
    ControllerDirection direction = update_direction(core);
    if (direction != NONE)
    {
      auto board_position = controller->board_position();
      auto board_position_change = _board_position_changes[direction];
      controller->board_position({
        board_position.first + board_position_change.first,
        board_position.second + board_position_change.second
      });
      
      controller->order(entity()->order() + board_position_change.first*10);
      
      board_position = controller->board_position();
      if (board_position.first < 0 ||
          board_position.first > 6 ||
          board_position.second < 0 ||
          board_position.second > board_position.first)
      {
        NotificationCenter::notify(DidJumpOff, *this);
      }
      NotificationCenter::notify(Event(DidJump, direction), *this);
    }
  }
}


//
// MARK: - ControllerAnimationComponent
//

// MARK: Helper functions

AnimationComponent::CubicHermiteSpline calculate_spline(Vector2 & end_point,
                                                        double duration,
                                                        Vector2 gravity)
{
  gravity *= PhysicsComponent::pixels_per_meter;
  const double t2 = duration*duration;
  const Vector2 m0 = end_point - gravity/2*t2;
  const Vector2 m1 = end_point + gravity/2*t2;
  return {{{0,0}, m0}, {end_point, m1}};
}

// MARK: Member functions

Vector2 * ControllerAnimationComponent::end_points()
{
  return _end_points;
}

ControllerAnimationComponent::ControllerAnimationComponent(Vector2 end_points[])
  : AnimationComponent()
{
  _end_points[0] = end_points[0];
  _end_points[1] = end_points[1];
  _end_points[2] = end_points[2];
  _end_points[3] = end_points[3];
}

void ControllerAnimationComponent::init(Entity * entity)
{
  AnimationComponent::init(entity);
  
  Vector2 gravity = entity->physics()->gravity();
  string ids[4] {"jump_up", "jump_down", "jump_left", "jump_right"};
  for (auto i = 0; i < 4; i++)
  {
    auto end_point = end_points()[i];
    auto id = ids[i];
    if (end_point.x != 0 || end_point.y != 0)
    {
      auto spline = calculate_spline(end_point, animation_speed(), gravity);
      addSegment(id, spline.first.first,  spline.first.second);
      addSegment(id, spline.second.first, spline.second.second);
    }
  }
    
  auto did_jump = [this](Event event)
  {
    switch (event.parameter())
    {
      case UP:
        performAnimation("jump_up", animation_speed(), _did_jump_off);
        break;
      case DOWN:
        performAnimation("jump_down", animation_speed(), _did_jump_off);
        break;
      case LEFT:
        performAnimation("jump_left", animation_speed(), _did_jump_off);
        break;
      case RIGHT:
        performAnimation("jump_right", animation_speed(), _did_jump_off);
        break;
    }
  };
  auto did_jump_off = [this](Event) { _did_jump_off = true; };
  
  auto input = entity->input();
  NotificationCenter::observe(did_jump, DidJump, input);
  NotificationCenter::observe(did_jump_off, DidJumpOff, input);
}

void ControllerAnimationComponent::reset()
{
  AnimationComponent::reset();
  
  _did_jump_off = false;
}


//
// MARK: - ControllerPhysicsComponent
//

// MARK: Member functions

void ControllerPhysicsComponent::init(Entity * entity)
{
  PhysicsComponent::init(entity);
  
  auto did_jump = [this](Event)
  {
    _has_jumped_once = true;
    dynamic(true);
  };
  auto did_jump_off = [this](Event)
  {
    collision_detection(false);
  };
  auto did_start_animating = [this](Event) { _animating = true;  };
  auto did_stop_animating  = [this](Event) { _animating = false; };
  
  auto input = entity->input();
  auto animation = entity->animation();
  NotificationCenter::observe(did_jump, DidJump, input);
  NotificationCenter::observe(did_jump_off, DidJumpOff, input);
  NotificationCenter::observe(did_start_animating,
                              DidStartAnimating,
                              animation);
  NotificationCenter::observe(did_stop_animating,
                              DidStopAnimating,
                              animation);
}

void ControllerPhysicsComponent::reset()
{
  PhysicsComponent::reset();
  
  _animating       = false;
  _has_jumped_once = false;
  dynamic(false);
  collision_detection(true);
  collision_response(true);
}

void ControllerPhysicsComponent::update(Core & core)
{
  PhysicsComponent::update(core);
  
  for (auto collided_entity : collided_entities())
  {
    collision(collided_entity);
  }
}


//
// MARK: - ControllerGraphicsComponent
//

// MARK: Member functions

void ControllerGraphicsComponent::init(Entity * entity)
{
  GraphicsComponent::init(entity);
  
  Controller * controller = (Controller*)entity;
  
  auto did_jump = [this, controller](Event event)
  {
    _current_direction = event.parameter();
    _jumping = true;
    const string prefix_jumping  = controller->prefix_jumping();
    const string id = prefix_jumping + "_" + to_string(_current_direction);
    current_sprite(SpriteCollection::main().retrieve(id));
  };
  
  auto did_stop_animating = [this, controller](Event)
  {
    _jumping = false;
    const string prefix_standing = controller->prefix_standing();
    const string id = prefix_standing + "_" + to_string(_current_direction);
    current_sprite(SpriteCollection::main().retrieve(id));
  };
  
  auto input     = entity->input();
  auto animation = entity->animation();
  NotificationCenter::observe(did_jump, DidJump, input);
  NotificationCenter::observe(did_stop_animating, DidStopAnimating, animation);
  
  resizeTo(16, 16);
}

void ControllerGraphicsComponent::reset()
{
  GraphicsComponent::reset();
  
  _current_direction = DOWN;
  _jumping = false;
  current_sprite(SpriteCollection::main().retrieve(default_sprite_id()));
}


//
// MARK: - Controller
//

Controller::Controller(string id, int order)
  : Entity(id, order)
{}

void Controller::init(Core * core)
{
  Entity::init(core);
  
  SpriteCollection & sprites = SpriteCollection::main();
  for (auto prefix : {prefix_standing(), prefix_jumping()})
  {
    int direction_mask = this->direction_mask();
    int direction = UP;
    while (direction_mask > 0)
    {
      if (direction_mask & 0b0001)
      {
        const string id = prefix + "_" + to_string(direction);
        const string filename = "textures/" + id + ".png";
        sprites.create(id, filename.c_str());
      }
      direction_mask >>= 1;
      direction++;
    }
  }
}
