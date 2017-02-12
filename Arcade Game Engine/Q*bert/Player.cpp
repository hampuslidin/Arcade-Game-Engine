//
//  Player.cpp
//  Game Engine
//

#include "Player.hpp"

/********************************
 * PlayerInputComponent
 ********************************/
void PlayerInputComponent::update(World & world)
{
  World::KeyStatus keys;
  world.getKeyStatus(keys);
  
  if (keys.up && !jumping())         notify(*entity(), DidJumpUp);
  else if (keys.down && !jumping())  notify(*entity(), DidJumpDown);
  else if (keys.left && !jumping())  notify(*entity(), DidJumpLeft);
  else if (keys.right && !jumping()) notify(*entity(), DidJumpRight);
  jumping(keys.up || keys.down || keys.left || keys.right);
}


/********************************
 * PlayerPhysicsComponent
 ********************************/
void PlayerPhysicsComponent::update(World & world)
{
  world.resolveCollisions(*entity());
}


/********************************
 * PlayerGraphicsComponent
 ********************************/
void PlayerGraphicsComponent::init(Entity * owner)
{
  Component::init(owner);
  Notifier * input_notifier = dynamic_cast<Notifier*>(this->entity()->input());
  if (input_notifier != 0)
  {
    auto events = {
      PlayerInputComponent::DidJumpUp,
      PlayerInputComponent::DidJumpDown,
      PlayerInputComponent::DidJumpLeft,
      PlayerInputComponent::DidJumpRight
    };
    input_notifier->addObserver(this, events);
  }
  resizeTo(32, 32);
}

void PlayerGraphicsComponent::update(World & world)
{
  if (sprites().size() == 0)
  {
    const std::vector<const char *> files {
      "textures/qbert_standing_down_right.png",
      "textures/qbert_standing_down_left.png",
      "textures/qbert_standing_up_right.png",
      "textures/qbert_standing_up_left.png",
      "textures/qbert_jumping_down_right.png",
      "textures/qbert_jumping_down_left.png",
      "textures/qbert_jumping_up_right.png",
      "textures/qbert_jumping_up_left.png",
    };
    initSprites(*world.renderer(), files);
  }
  Vector2 entity_pos = entity()->pos();
  int scale = world.scale();
  current_sprite()->draw(entity_pos.x + bounds().pos.x,
                         entity_pos.y + bounds().pos.y,
                         bounds().dim.w,
                         bounds().dim.h,
                         scale);
}

void PlayerGraphicsComponent::onNotify(Entity & entity, Event event)
{
  if (event == PlayerInputComponent::DidJumpRight) current_sprite(sprites()[0]);
  if (event == PlayerInputComponent::DidJumpDown)  current_sprite(sprites()[1]);
  if (event == PlayerInputComponent::DidJumpUp)    current_sprite(sprites()[2]);
  if (event == PlayerInputComponent::DidJumpLeft)  current_sprite(sprites()[3]);
}


/********************************
 * Player
 ********************************/
Player::Player()
  : Entity(new PlayerInputComponent(),
           new PlayerPhysicsComponent(),
           new PlayerGraphicsComponent())
{}

void Player::init(World * owner)
{
  Entity::init(owner);
  Rectangle screen_bounds = world()->bounds();
  moveTo(screen_bounds.dim.w/2, screen_bounds.dim.h/2);
}
