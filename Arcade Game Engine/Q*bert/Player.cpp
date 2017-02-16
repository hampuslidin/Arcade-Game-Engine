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
  
  jumping(false);
  
  const auto animation_notifier = dynamic_cast<Notifier*>(entity->animation());
  animation_notifier->addObserver(this, DidStopAnimating);
}

void PlayerInputComponent::update(Core & core)
{
  Player * const player = (Player*)(entity());
  Core::KeyStatus keys;
  core.getKeyStatus(keys);
  
  if (keys.up && !jumping())
  {
    notify(*player, Event(DidJump, UP));
  }
  else if (keys.down && !jumping())
  {
    notify(*player, Event(DidJump, DOWN));
  }
  else if (keys.left && !jumping())
  {
    notify(*player, Event(DidJump, LEFT));
  }
  else if (keys.right && !jumping())
  {
    notify(*player, Event(DidJump, RIGHT));
  }
  jumping(keys.up || keys.down || keys.left || keys.right);
}

void PlayerInputComponent::onNotify(Entity & entity, Event event)
{
  if (event == DidStopAnimating)
  {
    jumping(false);
  }
}

/********************************
 * PlayerAnimationComponent
 ********************************/
void PlayerAnimationComponent::init(Entity * entity)
{
  AnimationComponent::init(entity);
  
  loadAnimationFromFile("animations/jump_up.anim",    "jump_up",    0.55);
  loadAnimationFromFile("animations/jump_down.anim",  "jump_down",  0.55);
  loadAnimationFromFile("animations/jump_left.anim",  "jump_left",  0.55);
  loadAnimationFromFile("animations/jump_right.anim", "jump_right", 0.55);
  
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
        performAnimation("jump_up");
        break;
      case DOWN:
        performAnimation("jump_down");
        break;
      case LEFT:
        performAnimation("jump_left");
        break;
      case RIGHT:
        performAnimation("jump_right");
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
  collision_detection(true);
}

void PlayerPhysicsComponent::update(Core & core)
{
  PhysicsComponent::update(core);
  
  for (auto collided_entity : collided_entities())
  {
    string id_prefix = collided_entity->id().substr(0, 5);
    if (collided_entity->id().compare(0, 5, "block") == 0)
    {
      Block * block = dynamic_cast<Block*>(collided_entity);
      block->toggle(entity()->id());
      return;
    }
  }
  
  dynamic(true);
  collision_detection(false);
}


/********************************
 * PlayerGraphicsComponent
 ********************************/
void PlayerGraphicsComponent::init(Entity * entity)
{
  GraphicsComponent::init(entity);
  _current_direction = DOWN;
  _jumping = false;
  
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

void Player::init(Core * core)
{
  Entity::init(core);
  
  const Dimension2 view_dimensions = core->view_dimensions();
  moveTo(view_dimensions.x/2-8, view_dimensions.y-176-16-8);
}
