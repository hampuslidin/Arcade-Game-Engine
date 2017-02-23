//
//  Controller.hpp
//  Game Engine
//

#pragma once

#include "core.hpp"

// Events
const Event DidJump("DidJump");
const Event DidFallOff("DidFallOff");


//
// MARK: - ControllerDirection
//

typedef int ControllerDirection;
const ControllerDirection NONE  = -1;
const ControllerDirection UP    = 0;
const ControllerDirection DOWN  = 1;
const ControllerDirection LEFT  = 2;
const ControllerDirection RIGHT = 3;


//
// MARK: - ControllerInputComponent
//

class ControllerInputComponent
  : public InputComponent
{
  bool _animating;
  bool _did_jump_off;
protected:
  virtual ControllerDirection update_direction(Core & core) = 0;
  virtual double animation_ending_delay() = 0;
public:
  virtual void init(Entity * entity);
  virtual void reset();
  void update(Core & core);
};


//
// MARK: - ControllerAnimationComponent
//

class ControllerAnimationComponent
  : public AnimationComponent
{
protected:
  virtual double animation_speed() = 0;
  virtual Vector2 jump_up_end_point()    { return {}; };
  virtual Vector2 jump_down_end_point()  { return {}; };
  virtual Vector2 jump_left_end_point()  { return {}; };
  virtual Vector2 jump_right_end_point() { return {}; };
public:
  virtual void init(Entity * entity);
};


//
// MARK: - ControllerPhysicsComponent
//

class ControllerPhysicsComponent
  : public PhysicsComponent
{
  bool _animating;
  bool _did_fall_off;
protected:
  virtual bool should_break_for_collision(Entity * collided_entity) = 0;
  virtual bool should_update() = 0;
public:
  virtual void init(Entity * entity);
  virtual void reset();
  void update(Core & core);
};


//
// MARK: - ControllerGraphicsComponent
//

class ControllerGraphicsComponent
  : public GraphicsComponent
{
  int _current_direction;
  bool _jumping;
protected:
  virtual string default_sprite_id() = 0;
public:
  virtual void init(Entity * entity);
  virtual void reset();
};


//
// MARK: - Controller
//

class Controller : public Entity
{
protected:
  virtual int direction_mask() = 0;
public:
  Controller(string id,
             ControllerInputComponent * input,
             ControllerAnimationComponent * animation,
             ControllerPhysicsComponent * physics,
             ControllerGraphicsComponent * graphics);
  virtual void init(Core * core);
  virtual void reset();
  virtual string prefix_standing() = 0;
  virtual string prefix_jumping()  = 0;
};
