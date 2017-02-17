//
//  Player.cpp
//  Game Engine
//

#include "Player.hpp"
#include "Block.hpp"
#include <fstream>

/********************************
 * PlayerInputComponent
 ********************************/
void PlayerInputComponent::init(Entity * entity)
{
  InputComponent::init(entity);
  
  const auto animation_notifier = dynamic_cast<Notifier*>(entity->animation());
  animation_notifier->addObserver(this, DidStopAnimating);
  
  const auto physics_notifier = dynamic_cast<Notifier*>(entity->physics());
  physics_notifier->addObserver(this, DidFallOff);
}

void PlayerInputComponent::reset()
{
  InputComponent::reset();
  
  _animating = false;
  _did_jump_off = false;
}

void PlayerInputComponent::update(Core & core)
{
  if (!_animating && !_did_jump_off)
  {
    Core::KeyStatus keys;
    core.getKeyStatus(keys);
    
    if (keys.up)
    {
      notify(*entity(), Event(DidJump, UP));
    }
    else if (keys.down)
    {
      notify(*entity(), Event(DidJump, DOWN));
    }
    else if (keys.left)
    {
      notify(*entity(), Event(DidJump, LEFT));
    }
    else if (keys.right)
    {
      notify(*entity(), Event(DidJump, RIGHT));
    }
  }
}

void PlayerInputComponent::onNotify(Entity & entity, Event event)
{
  if (event == DidStopAnimating)
  {
    _animating = false;
  }
  else if (event == DidFallOff)
  {
    _did_jump_off = true;
  }
}

/********************************
 * PlayerAnimationComponent
 ********************************/
void PlayerAnimationComponent::init(Entity * entity)
{
  AnimationComponent::init(entity);
  
  loadAnimationFromFile("animations/jump_up.anim",    "jump_up",    0.5);
  loadAnimationFromFile("animations/jump_down.anim",  "jump_down",  0.5);
  loadAnimationFromFile("animations/jump_left.anim",  "jump_left",  0.5);
  loadAnimationFromFile("animations/jump_right.anim", "jump_right", 0.5);
  
  const auto input_notifier = dynamic_cast<Notifier*>(entity->input());
  input_notifier->addObserver(this, DidJump);
}

void PlayerAnimationComponent::onNotify(Entity & entity, Event event)
{
  if (event == DidJump)
  {
    switch (event.parameter())
    {
      case UP:
        performAnimation("jump_up", true);
        break;
      case DOWN:
        performAnimation("jump_down", true);
        break;
      case LEFT:
        performAnimation("jump_left", true);
        break;
      case RIGHT:
        performAnimation("jump_right", true);
        break;
    }
  }
}


/********************************
 * PlayerPhysicsComponent
 ********************************/
PlayerPhysicsComponent::PlayerPhysicsComponent()
{
  collision_bounds({7, 14, 2, 2});
}

void PlayerPhysicsComponent::init(Entity * entity)
{
  PhysicsComponent::init(entity);

  const auto input_notifier = dynamic_cast<Notifier*>(entity->input());
  input_notifier->addObserver(this, DidJump);
  
  const auto animation_notifier = dynamic_cast<Notifier*>(entity->animation());
  animation_notifier->addObserver(this, DidStartAnimating);
  animation_notifier->addObserver(this, DidStopAnimating);
  
  addObserver(this, DidMoveOutOfView);
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
  
  for (auto collided_entity : collided_entities())
  {
    string id_prefix = collided_entity->id().substr(0, 5);
    if (collided_entity->id().compare(0, 5, "block") == 0 && _has_jumped_once)
    {
      Block * block = dynamic_cast<Block*>(collided_entity);
      block->toggle(entity()->id());
      return;
    }
  }
  
  if (!_did_fall_off && !_animating && _has_jumped_once)
  {
    _did_fall_off = true;
    dynamic(true);
    collision_detection(false);
    entity()->order(0);
    notify(*entity(), DidFallOff);
  }
}

void PlayerPhysicsComponent::onNotify(Entity & entity, Event event)
{
  PhysicsComponent::onNotify(entity, event);
  
  if (event == DidJump)
  {
    _has_jumped_once = true;
    
    const auto input_notifier = dynamic_cast<Notifier*>(entity.input());
    input_notifier->removeObserver(this, DidJump);
  }
  else if (event == DidStartAnimating)
  {
    _animating = true;
  }
  else if (event == DidStopAnimating)
  {
    _animating = false;
  }
  else if (event == DidMoveOutOfView)
  {
    this->entity()->order(-1);
    this->entity()->core()->reset();
  }
}


/********************************
 * PlayerGraphicsComponent
 ********************************/
void PlayerGraphicsComponent::init(Entity * entity)
{
  GraphicsComponent::init(entity);
  
  vector<const char *> files {
    "textures/qbert_standing_up.png",
    "textures/qbert_standing_down.png",
    "textures/qbert_standing_left.png",
    "textures/qbert_standing_right.png",
    "textures/qbert_jumping_up.png",
    "textures/qbert_jumping_down.png",
    "textures/qbert_jumping_left.png",
    "textures/qbert_jumping_right.png",
  };
  initSprites(*entity->core()->renderer(), files, DOWN);
  
  const auto input_notifier = dynamic_cast<Notifier*>(entity->input());
  input_notifier->addObserver(this, DidJump);
  
  const auto animation_notifier = dynamic_cast<Notifier*>(entity->animation());
  animation_notifier->addObserver(this, DidStartMovingInAnimation);
  animation_notifier->addObserver(this, DidStopAnimating);
  
  resizeTo(16, 16);
}

void PlayerGraphicsComponent::reset()
{
  GraphicsComponent::reset();
  
  _current_direction = DOWN;
  _jumping = false;
  current_sprite(sprites()[DOWN]);
}

void PlayerGraphicsComponent::onNotify(Entity & entity, Event event)
{
  if (event == DidJump && !_jumping)
  {
    _current_direction = event.parameter();
    _jumping = true;
    current_sprite(sprites()[_current_direction]);
  }
  else if (event == DidStartMovingInAnimation)
  {
    current_sprite(sprites()[4+_current_direction]);
  }
  else if (event == DidStopAnimating)
  {
    _jumping = false;
    current_sprite(sprites()[_current_direction]);
  }
}


/********************************
 * Player
 ********************************/
Player::Player(string id)
  : Entity(id,
           new PlayerInputComponent(),
           new PlayerAnimationComponent(),
           new PlayerPhysicsComponent(),
           new PlayerGraphicsComponent())
{}

void Player::reset()
{
  Entity::reset();
  
  const Dimension2 view_dimensions = core()->view_dimensions();
  moveTo(view_dimensions.x/2-8, view_dimensions.y-176-16-8);
}
