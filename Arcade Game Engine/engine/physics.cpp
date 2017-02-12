//
//  physics.cpp
//  Game Engine
//

#include "core.hpp"


/********************************
 * World
 ********************************/
bool World::resolveCollisions(Entity & collider, bool collision_response)
{
  bool has_collided = false;
  if (collider.physics())
  {
    Rectangle collider_cb = collider.physics()->collision_bounds();
    for (auto entity : entities())
    {
      if (entity == &collider) continue;
      if (entity->physics())
      {
        Rectangle entity_cb = entity->physics()->collision_bounds();
        Vector2 c_pos = collider.position();
        Vector2 e_pos = entity->position();
        double left_overlap  = c_pos.x + max_x(collider_cb) -
        (e_pos.x + min_x(entity_cb));
        double up_overlap    = c_pos.y + max_y(collider_cb) -
        (e_pos.y + min_y(entity_cb));
        double right_overlap = e_pos.x + max_x(entity_cb)   -
        (c_pos.x + min_x(collider_cb));
        double down_overlap  = e_pos.y + max_y(entity_cb)   -
        (c_pos.y + min_y(collider_cb));
        
        if (left_overlap > 0 && up_overlap > 0 &&
            right_overlap > 0 && down_overlap > 0)
        {
          if (!collision_response) return true;
          has_collided = true;
          
          double * min = &up_overlap;
          const auto overlaps =
          {
            &left_overlap,
            &right_overlap,
            &down_overlap
          };
          for (auto f : overlaps) { if (*f >= *min) continue; min = f; }
          
          if (min == &left_overlap)
          {
            collider.moveBy(-left_overlap, 0);
            collider.changeHorizontalVelocityTo(0);
          }
          else if (min == &up_overlap)
          {
            collider.moveBy(0, -up_overlap);
            collider.changeVerticalVelocityTo(0);
          }
          else if (min == &right_overlap)
          {
            collider.moveBy(right_overlap, 0);
            collider.changeHorizontalVelocityTo(0);
          }
          else if (min == &down_overlap)
          {
            collider.moveBy(0, down_overlap);
            collider.changeVerticalVelocityTo(0);
          }
        }
      }
    }
  }
  return has_collided;
}


/********************************
 * PhysicsComponent
 ********************************/
PhysicsComponent::PhysicsComponent()
{
  dynamic(true);
  gravity(9.82);
  pixels_per_meter(200);
}

void PhysicsComponent::update(World & world)
{
  auto delta_time = world.delta_time();
  if (dynamic()) {
    entity()->changeVelocityBy(0, gravity() * delta_time * pixels_per_meter());
    auto distance = entity()->velocity() * delta_time;
    entity()->moveBy(distance.x, distance.y);
    world.resolveCollisions(*entity());
  }
  else
  {
    auto distance = entity()->velocity() * delta_time;
    entity()->moveBy(distance.x, distance.y);
  }
}
