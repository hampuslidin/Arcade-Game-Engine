//
//  physics.cpp
//  Game Engine
//

#include "core.hpp"

/********************************
 * Core
 ********************************/
void _resolveCollisions(Entity & collider,
                        Entity & collided,
                        bool collision_response,
                        vector<Entity*> & result)
{
  if (collider.physics() && collided.physics() && &collider != &collided)
  {
    Rectangle collider_cb = collider.physics()->collision_bounds();
    Rectangle collided_cb = collided.physics()->collision_bounds();
    Vector2 cr_pos, cd_pos;
    collider.calculateWorldPosition(cr_pos);
    collided.calculateWorldPosition(cd_pos);
    double left_overlap  =  cr_pos.x + max_x(collider_cb) -
                           (cd_pos.x + min_x(collided_cb));
    double up_overlap    =  cr_pos.y + max_y(collider_cb) -
                           (cd_pos.y + min_y(collided_cb));
    double right_overlap =  cd_pos.x + max_x(collided_cb) -
                           (cr_pos.x + min_x(collider_cb));
    double down_overlap  =  cd_pos.y + max_y(collided_cb) -
                           (cr_pos.y + min_y(collider_cb));
    
    if (left_overlap > 0 && up_overlap > 0 &&
        right_overlap > 0 && down_overlap > 0)
    {
      result.push_back(&collided);
      
      if (collision_response)
      {
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
  for (auto child : collided.children())
  {
    _resolveCollisions(collider, *child, collision_response, result);
  }
}

void Core::resolveCollisions(Entity & collider,
                             bool collision_response,
                             vector<Entity *> & result)
{
  _resolveCollisions(collider, *root(), collision_response, result);
}


//
// MARK: - PhysicsComponent
//

// MARK: Property functions

string PhysicsComponent::trait() { return "physics"; }

// MARK: Member functions

PhysicsComponent::PhysicsComponent()
{
  collision_bounds({0, 0, 16, 16});
  gravity(9.82);
  pixels_per_meter(90);
  dynamic(false);
  collision_detection(false);
  collision_response(false);
  simulate_with_animations(false);
}

void PhysicsComponent::init(Entity * entity)
{
  Component::init(entity);
  
  _should_simulate = true;
  _out_of_view = true;
  _did_collide = false;
  
  auto f1 = [this](Event, vector<GameObject*> *) { _should_simulate = false; };
  auto f2 = [this](Event, vector<GameObject*> *) { _should_simulate = true;  };
  
  NotificationCenter::main().observe(DidStartAnimating, f1);
  NotificationCenter::main().observe(DidStopAnimating,  f2);
}

void PhysicsComponent::update(Core & core)
{
  collided_entities().clear();
  if (_should_simulate || simulate_with_animations())
  {
    if (dynamic())
    {
      const auto vel_y = gravity() * core.delta_time() * pixels_per_meter();
      entity()->changeVelocityBy(0, vel_y);
      const auto distance = entity()->velocity() * core.delta_time();
      entity()->moveBy(distance.x, distance.y);
      
      Vector2 world_position;
      entity()->calculateWorldPosition(world_position);
      if ((world_position.x < 0 || world_position.y < 0 ||
          world_position.x >= core.view_dimensions().x ||
          world_position.y >= core.view_dimensions().y) && !_out_of_view)
      {
        _out_of_view = true;
        NotificationCenter::main().notify(DidMoveOutOfView);
      }
      else if ((world_position.x >= 0 &&
                world_position.x < core.view_dimensions().x &&
                world_position.y >= 0 &&
                world_position.y < core.view_dimensions().y) && _out_of_view)
      {
        _out_of_view = false;
        NotificationCenter::main().notify(DidMoveIntoView);
      }

    }
    
    if (collision_detection())
    {
      core.resolveCollisions(*entity(),
                             collision_response(),
                             collided_entities());
      if (collided_entities().size() > 0)
      {
        if (!_did_collide)
        {
          vector<GameObject*> game_objects(collided_entities().begin(),
                                           collided_entities().end());
          NotificationCenter::main().notify(DidCollide, &game_objects);
          _did_collide = true;
        }
      }
      else
      {
        _did_collide = false;
      }
    }
  }
  else
  {
    _did_collide = false;
  }
}
