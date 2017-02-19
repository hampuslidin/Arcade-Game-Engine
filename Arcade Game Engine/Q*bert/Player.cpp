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
  InputComponent::init(entity);
  
  auto f1 = [this](Event, vector<GameObject*> *) { _animating       = true;  };
  auto f2 = [this, entity](Event, vector<GameObject*> *)
  {
    entity->core()->createTimer(0.15, [this]()   { _animating       = false; });
  };
  auto f3 = [this](Event, vector<GameObject*> *) { _did_jump_off    = true;  };
  auto f4 = [this](Event, vector<GameObject*> *) { _did_clear_board = true;  };
  
  NotificationCenter::main().observe(DidStartAnimating, f1);
  NotificationCenter::main().observe(DidStopAnimating,  f2);
  NotificationCenter::main().observe(DidFallOff,        f3);
  NotificationCenter::main().observe(DidClearBoard,     f4);
}

void PlayerInputComponent::reset()
{
  InputComponent::reset();
  
  _animating = false;
  _did_jump_off = false;
  _did_clear_board = false;
}

void PlayerInputComponent::update(Core & core)
{
  if (!_animating && !_did_jump_off && !_did_clear_board)
  {
    Core::KeyStatus keys;
    core.getKeyStatus(keys);
    
    if (keys.up)
    {
      NotificationCenter::main().notify(Event(DidJump, UP));
    }
    else if (keys.down)
    {
      NotificationCenter::main().notify(Event(DidJump, DOWN));
    }
    else if (keys.left)
    {
      NotificationCenter::main().notify(Event(DidJump, LEFT));
    }
    else if (keys.right)
    {
      NotificationCenter::main().notify(Event(DidJump, RIGHT));
    }
  }
}


//
// MARK: - PlayerAnimationComponent
//

// MARK: Member functions

void PlayerAnimationComponent::init(Entity * entity)
{
  AnimationComponent::init(entity);
  
  addSegment("jump_up",    {0,     0}, {16,  -86.848});
  addSegment("jump_up",    {16,  -24}, {16,   38.848});
  
  addSegment("jump_down",  {0,     0}, {-16, -38.848});
  addSegment("jump_down",  {-16,  24}, {-16,  86.848});
  
  addSegment("jump_left",  {0,     0}, {-16, -86.848});
  addSegment("jump_left",  {-16, -24}, {-16,  38.848});
  
  addSegment("jump_right", {0,     0}, {16,  -38.848});
  addSegment("jump_right", {16,   24}, {16,   86.848});
  
  auto f = [this](Event event, vector<GameObject*> *)
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
  
  NotificationCenter::main().observe(DidJump, f);
}


//
// MARK: - PlayerPhysicsComponent
//

// MARK: Member functions

PlayerPhysicsComponent::PlayerPhysicsComponent()
{
  collision_bounds({7, 14, 2, 2});
}

void PlayerPhysicsComponent::init(Entity * entity)
{
  PhysicsComponent::init(entity);

  auto f1 = [this](Event, vector<GameObject*> *) { _has_jumped_once = true;  };
  auto f2 = [this](Event, vector<GameObject*> *) { _animating       = true;  };
  auto f3 = [this](Event, vector<GameObject*> *) { _animating       = false; };
  auto f4 = [entity](Event, vector<GameObject*> *)
  {
    entity->order(-1);
    entity->core()->createTimer(1, [entity]()
    {
      entity->core()->reset();
    });
  };
  
  NotificationCenter::main().observe(DidJump,           f1);
  NotificationCenter::main().observe(DidStartAnimating, f2);
  NotificationCenter::main().observe(DidStopAnimating,  f3);
  NotificationCenter::main().observe(DidMoveOutOfView,  f4);
}

void PlayerPhysicsComponent::reset()
{
  PhysicsComponent::reset();
  
  _has_jumped_once = false;
  _animating       = false;
  _did_fall_off    = false;
  dynamic(false);
  collision_detection(true);
}

void PlayerPhysicsComponent::update(Core & core)
{
  PhysicsComponent::update(core);
  
  if (_has_jumped_once)
  {
    for (auto collided_entity : collided_entities())
    {
      string id_prefix = collided_entity->id().substr(0, 5);
      if (collided_entity->id().compare(0, 5, "block") == 0)
      {
        ((Block*)collided_entity)->toggle_state();
        return;
      }
    }
    
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
// MARK: - PlayerGraphicsComponent
//

// MARK: Member functions

void PlayerGraphicsComponent::init(Entity * entity)
{
  GraphicsComponent::init(entity);
  
  auto f1 = [this](Event event, vector<GameObject*> *)
  {
    _current_direction = event.parameter();
    _jumping = true;
    const string id = "qbert_standing_" + to_string(_current_direction);
    current_sprite(SpriteCollection::main().retrieve(id));
  };
  
  auto f2 = [this](Event, vector<GameObject*> *)
  {
    const string id = "qbert_jumping_" + to_string(_current_direction);
    current_sprite(SpriteCollection::main().retrieve(id));
  };
  
  auto f3 = [this](Event, vector<GameObject*> *)
  {
    _jumping = false;
    const string id = "qbert_standing_" + to_string(_current_direction);
    current_sprite(SpriteCollection::main().retrieve(id));
  };
  
  NotificationCenter::main().observe(DidJump,                    f1);
  NotificationCenter::main().observe(DidStartMovingInAnimation,  f2);
  NotificationCenter::main().observe(DidStopAnimating,           f3);
  
  resizeTo(16, 16);
}

void PlayerGraphicsComponent::reset()
{
  GraphicsComponent::reset();
  
  _current_direction = DOWN;
  _jumping = false;
  current_sprite(SpriteCollection::main().retrieve("qbert_standing_1"));
}


//
// MARK: - Player
//

Player::Player(string id)
  : Entity(id,
           new PlayerInputComponent(),
           new PlayerAnimationComponent(),
           new PlayerPhysicsComponent(),
           new PlayerGraphicsComponent())
{}

void Player::init(Core * core)
{
  Entity::init(core);
  
  SpriteCollection & sprites = SpriteCollection::main();
  string postures[]   {"standing", "jumping"};
  for (auto i = 0; i < 2; i++)
  {
    string posture = postures[i];
    for (auto j = 0; j < 4; j++)
    {
      string id = "qbert_" + posture + "_" + to_string(j);
      string filename = "textures/" + id + ".png";
      sprites.create(id, filename.c_str());
    }
  }
}

void Player::reset()
{
  Entity::reset();
  
  const Dimension2 view_dimensions = core()->view_dimensions();
  moveTo(view_dimensions.x/2-8, view_dimensions.y-176-16-8);
}
