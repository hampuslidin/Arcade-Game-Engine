//
//  Player.cpp
//  Game Engine
//

#include "Player.hpp"
#include <fstream>

/********************************
 * PlayerInputComponent
 ********************************/
void PlayerInputComponent::init(Entity * owner)
{
  Component::init(owner);
  
  jumping(false);
  
  const auto animation_notifier =
    dynamic_cast<Notifier*>(this->entity()->animation());
  animation_notifier->addObserver(this, DidStopAnimating);
}



void PlayerInputComponent::update(World & world)
{
  Player * const player = (Player*)(entity());
  World::KeyStatus keys;
  world.getKeyStatus(keys);
  
  if (keys.up && !jumping())
  {
    notify(*player, Event(DidJump, UP));
  }
  if (keys.down && !jumping())
  {
    notify(*player, Event(DidJump, DOWN));
  }
  if (keys.left && !jumping())
  {
    notify(*player, Event(DidJump, LEFT));
  }
  if (keys.right && !jumping())
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
void PlayerAnimationComponent::init(Entity * owner)
{
  Component::init(owner);
  
  loadAnimationFromFile("animations/jump_up.anim",    "jump_up",    0.5);
  loadAnimationFromFile("animations/jump_down.anim",  "jump_down",  0.5);
  loadAnimationFromFile("animations/jump_left.anim",  "jump_left",  0.5);
  loadAnimationFromFile("animations/jump_right.anim", "jump_right", 0.5);
  
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
void PlayerGraphicsComponent::init(Entity * owner)
{
  Component::init(owner);
  
  const auto input_notifier = dynamic_cast<Notifier*>(this->entity()->input());
  input_notifier->addObserver(this, DidJump);
  
  resizeTo(16, 16);
}

void PlayerGraphicsComponent::update(World & world)
{
  if (sprites().size() == 0)
  {
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
    initSprites(*world.renderer(), files, DOWN);
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
  if (sprites().size() && event == DidJump)
  {
    current_sprite(sprites()[event.parameter()]);
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

void Player::init(World * owner)
{
  Entity::init(owner);
  Dimension2 view_dimensions = world()->view_dimensions();
  moveTo(view_dimensions.w/2, view_dimensions.h/2);
}
