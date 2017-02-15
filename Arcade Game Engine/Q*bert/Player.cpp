//
//  Player.cpp
//  Game Engine
//

#include "Player.hpp"
#include <fstream>

/********************************
 * PlayerInputComponent
 ********************************/
void PlayerInputComponent::init(Entity * entity)
{
  Component::init(entity);
  
  jumping(false);
  
  const auto animation_notifier =
    dynamic_cast<Notifier*>(this->entity()->animation());
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
  Component::init(entity);
  
  loadAnimationFromFile("animations/jump_up.anim",    "jump_up",    0.55);
  loadAnimationFromFile("animations/jump_down.anim",  "jump_down",  0.55);
  loadAnimationFromFile("animations/jump_left.anim",  "jump_left",  0.55);
  loadAnimationFromFile("animations/jump_right.anim", "jump_right", 0.55);
  
  const auto input_notifier = dynamic_cast<Notifier*>(this->entity()->input());
  input_notifier->addObserver(this, DidJump);
}

void PlayerAnimationComponent::onNotify(Entity & entity, Event event)
{
  if (event == DidJump)
  {
    const char * id;
    switch (event.parameter())
    {
      case UP:
        id = "jump_up";
        break;
      case DOWN:
        id = "jump_down";
        break;
      case LEFT:
        id = "jump_left";
        break;
      case RIGHT:
        id = "jump_right";
        break;
    }
    performAnimation(id);
  }
}

/********************************
 * PlayerPhysicsComponent
 ********************************/
PlayerPhysicsComponent::PlayerPhysicsComponent()
  : PhysicsComponent()
{
  collision_bounds({0, 0, 16, 16});
  dynamic(false);
}

/********************************
 * PlayerGraphicsComponent
 ********************************/
void PlayerGraphicsComponent::init(Entity * entity)
{
  GraphicsComponent::init(entity);
  _current_direction = DOWN;
  _jumping = false;
  
  const std::vector<const char *> files {
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
  
  const auto input_notifier = dynamic_cast<Notifier*>(this->entity()->input());
  input_notifier->addObserver(this, DidJump);
  
  const auto animation_notifier =
    dynamic_cast<Notifier*>(this->entity()->animation());
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
Player::Player()
  : Entity(new PlayerInputComponent(),
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
