//
//  Controller.hpp
//  Game Engine
//

#pragma once

#include "core.hpp"
#include "Board.hpp"

// Events
const Event DidJump("DidJump");
const Event DidJumpOff("DidJumpOff");
const Event DidCollideWithBlock("DidCollideWithBlock");
const Event DidCollideWithEnemy("DidCollideWithEnemy");


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
protected:
  prop_r<ControllerInputComponent, bool> airborn;
  
  virtual ControllerDirection update_direction(Core & core) = 0;
  virtual double animation_ending_delay() = 0;
  virtual vector<pair<int, int>> board_position_changes() = 0;
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
  bool _did_jump_off;
protected:
  virtual vector<Vector2> end_points() = 0;
  virtual double animation_speed() = 0;
public:
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
  virtual void collision_with_block(Block * block) {};
  virtual void collision_with_entity(Entity * entity) {};
public:
  virtual void init(Entity * entity);
  virtual void reset();
  void update(Core & core);
};


//
// MARK: - ControllerAudioComponent
//

typedef AudioComponent ControllerAudioComponent;


//
// MARK: - ControllerGraphicsComponent
//

class ControllerGraphicsComponent
  : public GraphicsComponent
{
  int _current_direction;
  bool _jumping;
protected:
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
  virtual pair<int, int> default_board_position() = 0;
  virtual int default_order() = 0;
  virtual ControllerDirection default_direction() = 0;
public:
  prop<     pair<int, int>> previous_board_position;
  prop<     pair<int, int>> board_position;
  prop<                int> previous_order;
  prop<ControllerDirection> direction;
  
  Controller(string id, int order);
  virtual void init(Core * core);
  virtual string prefix_standing() = 0;
  virtual string prefix_jumping()  = 0;
};
