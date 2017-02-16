//
//  physics.cpp
//  Game Engine
//

#include "core.hpp"


/********************************
 * Core
 ********************************/
void Core::resolveCollisions(Entity & collider,
                             bool collision_response,
                             vector<Entity*> & result)
{
  if (collider.physics())
  {
    Rectangle collider_cb = collider.physics()->collision_bounds();
    for (auto entity : _entities)
    {
      if (entity == &collider) continue;
      if (entity->physics())
      {
        Rectangle entity_cb = entity->physics()->collision_bounds();
        Vector2 c_pos, e_pos;
        collider.calculateWorldPosition(c_pos);
        entity->calculateWorldPosition(e_pos);
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
          result.push_back(entity);
          
          if (!collision_response) continue;
          
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
}


/********************************
 * PhysicsComponent
 ********************************/
PhysicsComponent::PhysicsComponent()
{
  _should_simulate = true;
  collision_bounds({0, 0, 16, 16});
  gravity(9.82);
  pixels_per_meter(200);
  dynamic(false);
  collision_detection(false);
  collision_response(false);
  simulate_with_animations(false);
}

void PhysicsComponent::init(Entity * entity)
{
  Component::init(entity);
  
  const auto animation_notifier = dynamic_cast<Notifier*>(entity->animation());
  if (animation_notifier)
  {
    animation_notifier->addObserver(this, DidStartAnimating);
    animation_notifier->addObserver(this, DidStopAnimating);
  }
}

void PhysicsComponent::update(Core & core)
{
  if (_should_simulate || simulate_with_animations())
  {
    auto delta_time = core.delta_time();
    
    if (dynamic())
    {
      entity()->changeVelocityBy(0,
                                 gravity() * delta_time * pixels_per_meter());
    }
    
    if (collision_detection())
    {
      collided_entities().clear();
      core.resolveCollisions(*entity(),
                             collision_response(),
                             collided_entities());
      if (collided_entities().size() > 0)
      {
        notify(*entity(), DidCollide);
      }
    }
    
    auto distance = entity()->velocity() * delta_time;
    entity()->moveBy(distance.x, distance.y);
  }
}

void PhysicsComponent::onNotify(Entity & entity, Event event)
{
  if (event == DidStartAnimating)
  {
    _should_simulate = false;
  }
  else if (event == DidStopAnimating)
  {
    _should_simulate = true;
  }
}
