//
//  Player.cpp
//  Game Engine
//

#include "Player.hpp"

/********************************
 * PlayerInputComponent
 ********************************/
PlayerInputComponent::PlayerInputComponent()
{
  jumping(false);
}

void PlayerInputComponent::update(World & world)
{
  Player * const player = (Player*)(entity());
  World::KeyStatus keys;
  world.getKeyStatus(keys);
  
  const Vector2 speed = player->speed();
  if (keys.up && !jumping())
  {
    entity()->changeVerticalVelocityTo(-speed.y);
    notify(*player, DidJumpUp);
  }
  if (keys.down && !jumping())
  {
    entity()->changeVerticalVelocityTo(speed.y);
    notify(*player, DidJumpDown);
  }
  if (keys.left && !jumping())
  {
    player->changeHorizontalVelocityTo(-speed.x);
    notify(*player, DidJumpLeft);
  }
  if (keys.right && !jumping())
  {
    player->changeHorizontalVelocityTo(speed.x);
    notify(*entity(), DidJumpRight);
  }
  if (!(keys.left || keys.right))
  {
    jumping(false);
    auto velocity_y = player->velocity().y;
    player->changeVelocityTo(0, velocity_y);
  }
}

/********************************
 * PlayerPhysicsComponent
 ********************************/
PlayerPhysicsComponent::PlayerPhysicsComponent()
  : PhysicsComponent()
{
  collision_bounds({0, 0, 16, 16});
}

/********************************
 * PlayerGraphicsComponent
 ********************************/
void PlayerGraphicsComponent::init(Entity * owner)
{
  Component::init(owner);
  
  auto events = {
    PlayerInputComponent::DidJumpUp,
    PlayerInputComponent::DidJumpDown,
    PlayerInputComponent::DidJumpLeft,
    PlayerInputComponent::DidJumpRight
  };
  dynamic_cast<Notifier*>(this->entity()->input())->addObserver(this, events);
  
  resizeTo(16, 16);
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
  Vector2 entity_pos = entity()->position();
  int scale = world.scale();
  current_sprite()->draw(entity_pos.x + bounds().pos.x,
                         entity_pos.y + bounds().pos.y,
                         bounds().dim.w,
                         bounds().dim.h,
                         scale);
}

void PlayerGraphicsComponent::onNotify(Entity & entity, Event event)
{
  if (sprites().size())
  {
    if (event == PlayerInputComponent::DidJumpRight) {
      current_sprite(sprites()[0]);
    }
    if (event == PlayerInputComponent::DidJumpDown)
    {
      current_sprite(sprites()[1]);
    }
    if (event == PlayerInputComponent::DidJumpUp)
    {
      current_sprite(sprites()[2]);
    }
    if (event == PlayerInputComponent::DidJumpLeft)
    {
      current_sprite(sprites()[3]);
    }
  }
}


/********************************
 * Player
 ********************************/
Player::Player()
  : Entity(new PlayerInputComponent(),
           new PlayerPhysicsComponent(),
           new PlayerGraphicsComponent())
{
  speed({100, 200});
}

void Player::init(World * owner)
{
  Entity::init(owner);
  Rectangle screen_bounds = world()->bounds();
  moveTo(screen_bounds.dim.w/2, screen_bounds.dim.h/2);
}
