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
  
  auto did_start_animating = [this](Event, vector<GameObject*> *)
  {
    _animating = true;
  };
  auto did_stop_animating = [this, entity](Event, vector<GameObject*> *)
  {
    entity->core()->createTimer(0.15, [this]() { _animating = false; });
  };
  auto did_fall_off = [this](Event, vector<GameObject*> *)
  {
    _did_jump_off = true;
  };
  
  NotificationCenter::main().observe(DidStartAnimating, did_start_animating);
  NotificationCenter::main().observe(DidStopAnimating,  did_stop_animating);
  NotificationCenter::main().observe(DidFallOff,        did_fall_off);
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
        NotificationCenter::main().notify(Event(DidJump, direction));
        break;
    }
  }
}


//
// MARK: - ControllerAnimationComponent
//

// MARK: Member functions

void ControllerAnimationComponent::init(Entity * entity)
{
  AnimationComponent::init(entity);
  
  auto did_jump = [this](Event event, vector<GameObject*> *)
  {
    switch (event.parameter())
    {
      case UP:
        performAnimation("jump_up", 0.4, true);
        break;
      case DOWN:
        performAnimation("jump_down", 0.4, true);
        break;
      case LEFT:
        performAnimation("jump_left", 0.4, true);
        break;
      case RIGHT:
        performAnimation("jump_right", 0.4, true);
        break;
    }
  };
  
  NotificationCenter::main().observe(DidJump, did_jump);
}


//
// MARK: - ControllerPhysicsComponent
//

// MARK: Member functions

void ControllerPhysicsComponent::init(Entity * entity)
{
  PhysicsComponent::init(entity);
  
  auto did_start_animating = [this](Event, vector<GameObject*> *)
  {
    _animating       = true;
  };
  auto did_stop_animating = [this](Event, vector<GameObject*> *)
  {
    _animating       = false;
  };
  
  NotificationCenter::main().observe(DidStartAnimating, did_start_animating);
  NotificationCenter::main().observe(DidStopAnimating,  did_stop_animating);
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
      bool should_break = collision_event(collided_entity);
      if (should_break) break;
    }
    if (collided_entities().size() > 0) return;
    
    if (!_did_fall_off && !_animating)
    {
      _did_fall_off = true;
      dynamic(true);
      collision_detection(false);
      entity()->order(0);
      NotificationCenter::main().notify(DidFallOff);
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
  
  auto did_jump = [this, controller](Event event, vector<GameObject*> *)
  {
    _current_direction = event.parameter();
    _jumping = true;
    const string prefix_jumping  = controller->prefix_jumping();
    const string id = prefix_jumping + "_" + to_string(_current_direction);
    current_sprite(SpriteCollection::main().retrieve(id));
  };
  
  auto did_stop_animating = [this, controller](Event, vector<GameObject*> *)
  {
    _jumping = false;
    const string prefix_standing = controller->prefix_standing();
    const string id = prefix_standing + "_" + to_string(_current_direction);
    current_sprite(SpriteCollection::main().retrieve(id));
  };
  
  NotificationCenter::main().observe(DidJump,           did_jump);
  NotificationCenter::main().observe(DidStopAnimating,  did_stop_animating);
  
  resizeTo(16, 16);
}

void ControllerGraphicsComponent::reset()
{
  GraphicsComponent::reset();
  
  _current_direction = DOWN;
  _jumping = false;
  current_sprite(SpriteCollection::main().retrieve("qbert_standing_1"));
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
  const string prefixes[] {prefix_standing(), prefix_jumping()};
  for (auto i = 0; i < 2; i++)
  {
    const string prefix = prefixes[i];
    for (auto j = 0; j < 4; j++)
    {
      const string id = prefix + "_" + to_string(j);
      const string filename = "textures/" + id + ".png";
      sprites.create(id, filename.c_str());
    }
  }
}

void Controller::reset()
{
  Entity::reset();
  
  const Dimension2 view_dimensions = core()->view_dimensions();
  moveTo(view_dimensions.x/2-8, view_dimensions.y-176-16-8);
}
