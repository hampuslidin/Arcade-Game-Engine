//
//  physics.cpp
//  Game Engine
//

#include "core.hpp"


/********************************
 * PhysicsComponent
 ********************************/
PhysicsComponent::PhysicsComponent()
{
  dynamic(true);
  gravity(500);
}

void PhysicsComponent::update(World & world)
{
  if (dynamic()) {
    entity()->changeVelocityBy(0, gravity() * world.delta_time());
    auto distance = entity()->velocity() * world.delta_time();
    entity()->moveBy(distance.x, distance.y);
    world.resolveCollisions(*entity());
  }
  else
  {
    auto distance = entity()->velocity() * world.delta_time();
    entity()->moveBy(distance.x, distance.y);
  }
}
