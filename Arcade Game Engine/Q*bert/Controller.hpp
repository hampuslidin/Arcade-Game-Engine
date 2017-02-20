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
  virtual bool collision_event(Entity * collided_entity) = 0;
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
public:
  virtual void init(Entity * entity);
  virtual void reset();
};


//
// MARK: - Controller
//

class Controller : public Entity
{
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
