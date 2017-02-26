//
//  Controller.hpp
//  Game Engine
//

#pragma once

#include "core.hpp"

// Events
const Event DidJump("DidJump");
const Event DidJumpOff("DidJumpOff");


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
  pair<int, int> _board_position_changes[4];
  bool _animating;
  bool _airborn;
protected:
  virtual ControllerDirection update_direction(Core & core) = 0;
  virtual double animation_ending_delay() = 0;
public:
  ControllerInputComponent(pair<int, int> board_position_changes[4]);
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
  Vector2 _end_points[4];
  bool _did_jump_off;
protected:
  Vector2 * end_points();
  virtual double animation_speed() = 0;
public:
  ControllerAnimationComponent(Vector2 end_points[4]);
  virtual void init(Entity * entity);
  virtual void reset();
};


//
// MARK: - ControllerPhysicsComponent
//

class ControllerPhysicsComponent
  : public PhysicsComponent
{
  bool _animating;
  bool _has_jumped_once;
protected:
  virtual bool should_break_for_collision(Entity * collided_entity) = 0;
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
  virtual int default_order() = 0;
  virtual pair<int, int> default_board_position() = 0;
public:
  prop<pair<int, int>> board_position;
  
  Controller(string id,
             int order,
             ControllerInputComponent * input,
             ControllerAnimationComponent * animation,
             ControllerPhysicsComponent * physics,
             ControllerGraphicsComponent * graphics);
  virtual void init(Core * core);
  virtual void reset();
  virtual string prefix_standing() = 0;
  virtual string prefix_jumping()  = 0;
};
