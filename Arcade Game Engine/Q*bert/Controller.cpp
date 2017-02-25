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

void ControllerInputComponent::init(Entity * entity)
{
  InputComponent::init(entity);
  
  auto did_start_animating = [this](Event) { _animating = true; };
  auto did_stop_animating  = [this, entity](Event)
  {
    entity->core()->createTimer(animation_ending_delay(), [this]
    {
      _animating = false;
    });
  };
  auto did_fall_off = [this](Event) { _did_jump_off = true; };
  
  auto animation = entity->animation();
  auto physics   = entity->physics();
  NotificationCenter::observe(did_start_animating,
                              DidStartAnimating,
                              animation);
  NotificationCenter::observe(did_stop_animating,
                              DidStopAnimating,
                              animation);
  NotificationCenter::observe(did_fall_off,
                              DidFallOff,
                              physics);
}

void ControllerInputComponent::reset()
{
  InputComponent::reset();
  
  _animating = false;
  _did_jump_off = false;
}

void ControllerInputComponent::update(Core & core)
{
  if (!_animating && !_did_jump_off)
  {
    ControllerDirection direction;
    switch (direction = update_direction(core))
    {
      case UP:
      case DOWN:
      case LEFT:
      case RIGHT:
        NotificationCenter::notify(Event(DidJump, direction), *this);
        break;
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

void ControllerAnimationComponent::init(Entity * entity)
{
  AnimationComponent::init(entity);
  
  Vector2 gravity = entity->physics()->gravity();
  Vector2 end_point = jump_up_end_point();
  if (end_point.x != 0 || end_point.y != 0)
  {
    auto spline = calculate_spline(end_point, animation_speed(), gravity);
    addSegment("jump_up", spline.first.first,  spline.first.second);
    addSegment("jump_up", spline.second.first, spline.second.second);
  }
  
  end_point = jump_down_end_point();
  if (end_point.x != 0 || end_point.y != 0)
  {
    auto spline = calculate_spline(end_point, animation_speed(), gravity);
    addSegment("jump_down", spline.first.first,  spline.first.second);
    addSegment("jump_down", spline.second.first, spline.second.second);
  }
  
  end_point = jump_left_end_point();
  if (end_point.x != 0 || end_point.y != 0)
  {
    auto spline = calculate_spline(end_point, animation_speed(), gravity);
    addSegment("jump_left", spline.first.first,  spline.first.second);
    addSegment("jump_left", spline.second.first, spline.second.second);
  }
  
  end_point = jump_right_end_point();
  if (end_point.x != 0 || end_point.y != 0)
  {
    auto spline = calculate_spline(end_point, animation_speed(), gravity);
    addSegment("jump_right", spline.first.first,  spline.first.second);
    addSegment("jump_right", spline.second.first, spline.second.second);
  }
  
  auto did_jump = [this](Event event)
  {
    switch (event.parameter())
    {
      case UP:
        performAnimation("jump_up", animation_speed(), true);
        break;
      case DOWN:
        performAnimation("jump_down", animation_speed(), true);
        break;
      case LEFT:
        performAnimation("jump_left", animation_speed(), true);
        break;
      case RIGHT:
        performAnimation("jump_right", animation_speed(), true);
        break;
    }
  };
  
  auto input = entity->input();
  NotificationCenter::observe(did_jump, DidJump, input);
}


//
// MARK: - ControllerPhysicsComponent
//

// MARK: Member functions

void ControllerPhysicsComponent::init(Entity * entity)
{
  PhysicsComponent::init(entity);
  
  auto did_start_animating = [this](Event) { _animating = true;  };
  auto did_stop_animating  = [this](Event) { _animating = false; };
  
  auto animation = entity->animation();
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
  _did_fall_off    = false;
  dynamic(false);
  collision_detection(true);
}

void ControllerPhysicsComponent::update(Core & core)
{
  PhysicsComponent::update(core);
  
  if (should_update())
  {
    for (auto collided_entity : collided_entities())
    {
      bool should_break = should_break_for_collision(collided_entity);
      if (should_break) break;
    }
    if (collided_entities().size() > 0) return;
    
    if (!_did_fall_off && !_animating)
    {
      _did_fall_off = true;
      dynamic(true);
      collision_detection(false);
      entity()->order(0);
      NotificationCenter::notify(DidFallOff, *this);
    }
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

Controller::Controller(string id,
                       ControllerInputComponent * input,
                       ControllerAnimationComponent * animation,
                       ControllerPhysicsComponent * physics,
                       ControllerGraphicsComponent * graphics)
  : Entity(id, input, animation, physics, graphics)
{}

void Controller::init(Core * core)
{
  Entity::init(core);
  
  SpriteCollection & sprites = SpriteCollection::main();
  for (auto prefix : {prefix_standing(), prefix_jumping()})
  {
    int direction_mask = this->direction_mask();
    int direction = RIGHT;
    while (direction_mask > 0)
    {
      if (direction_mask & 0b0001)
      {
        const string id = prefix + "_" + to_string(direction);
        const string filename = "textures/" + id + ".png";
        sprites.create(id, filename.c_str());
      }
      direction_mask >>= 1;
      direction--;
    }
  }
}

void Controller::reset()
{
  Entity::reset();
  
  const Dimension2 view_dimensions = core()->view_dimensions();
  moveTo(view_dimensions.x/2-8, view_dimensions.y-176-16-8);
}
