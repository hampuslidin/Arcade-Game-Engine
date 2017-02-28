//
//  physics.cpp
//  Game Engine
//

#include <algorithm>
#include "core.hpp"


//
// MARK: - Core
//

void _resolveCollisions(Entity & collider,
                        Entity & obsticle,
                        Vector2 & travel_distance,
                        bool collision_response,
                        vector<Entity*> & result)
{
  if (collider.physics() && obsticle.physics() && &collider != &obsticle)
  {
    Rectangle collider_cb = collider.physics()->collision_bounds();
    Rectangle obsticle_cb = obsticle.physics()->collision_bounds();
    Vector2 col_pos, obs_pos;
    collider.calculateWorldPosition(col_pos);
    obsticle.calculateWorldPosition(obs_pos);
    
    SDL_Rect collider_before_rect
    {
      (int)(col_pos.x + collider_cb.pos.x),
      (int)(col_pos.y + collider_cb.pos.y),
      (int)collider_cb.dim.x,
      (int)collider_cb.dim.y
    };
    
    SDL_Rect obsticle_rect
    {
      (int)(obs_pos.x + obsticle_cb.pos.x),
      (int)(obs_pos.y + obsticle_cb.pos.y),
      (int)obsticle_cb.dim.x,
      (int)obsticle_cb.dim.y
    };
    
    // second condition is a hack for enemies not colliding away from each other
    bool is_responding = collision_response && !obsticle.physics()->dynamic();
    
    // check so that the collider is not already colliding with obsticle
    SDL_Rect intersection_rect;
    if (!SDL_IntersectRect(&collider_before_rect,
                           &obsticle_rect,
                           &intersection_rect))
    {
      // collider is outside of obsticle
      SDL_Rect collider_after_rect
      {
        (int)(col_pos.x + collider_cb.pos.x + travel_distance.x),
        (int)(col_pos.y + collider_cb.pos.y + travel_distance.y),
        (int)collider_cb.dim.x,
        (int)collider_cb.dim.y
      };
      
      // the bounding box for the colliders before and after traviling the
      // distance
      SDL_Rect large_rect;
      large_rect.x = (int)min(min_x(collider_before_rect),
                              min_x(collider_after_rect));
      large_rect.y = (int)min(min_y(collider_before_rect),
                              min_y(collider_after_rect));
      large_rect.w = (int)max(max_x(collider_before_rect),
                              max_x(collider_after_rect)) - large_rect.x;
      large_rect.h = (int)max(max_y(collider_before_rect),
                              max_y(collider_after_rect)) - large_rect.y;
      
      if (SDL_IntersectRect(&large_rect, &obsticle_rect, &intersection_rect))
      {
        // collider will collide with obsticle between this frame and the next
        result.push_back(&obsticle);
        
        if (is_responding)
        {
          typedef struct { int x1, y1, x2, y2; } Line;
          
          auto distance = [](int x1, int y1, int x2, int y2)
          {
            double dx = x2-x1;
            double dy = y2-y1;
            return sqrt(dx*dx+dy*dy);
          };
          
          Line upper_left, upper_right, lower_left, lower_right;
          
          upper_left.x1  = min_x(collider_before_rect);
          upper_left.y1  = min_y(collider_before_rect);
          upper_left.x2  = min_x(collider_after_rect);
          upper_left.y2  = min_y(collider_after_rect);
          
          upper_right.x1 = max_x(collider_before_rect);
          upper_right.y1 = min_y(collider_before_rect);
          upper_right.x2 = max_x(collider_after_rect);
          upper_right.y2 = min_y(collider_after_rect);
          
          lower_left.x1  = min_x(collider_before_rect);
          lower_left.y1  = max_y(collider_before_rect);
          lower_left.x2  = min_x(collider_after_rect);
          lower_left.y2  = max_y(collider_after_rect);
          
          lower_right.x1 = max_x(collider_before_rect);
          lower_right.y1 = max_y(collider_before_rect);
          lower_right.x2 = max_x(collider_after_rect);
          lower_right.y2 = max_y(collider_after_rect);
          
          bool is_intersecting = false;
          is_intersecting |= SDL_IntersectRectAndLine(&obsticle_rect,
                                                      &upper_left.x1,
                                                      &upper_left.y1,
                                                      &upper_left.x2,
                                                      &upper_left.y2);
          is_intersecting |= SDL_IntersectRectAndLine(&obsticle_rect,
                                                      &upper_right.x1,
                                                      &upper_right.y1,
                                                      &upper_right.x2,
                                                      &upper_right.y2);
          is_intersecting |= SDL_IntersectRectAndLine(&obsticle_rect,
                                                      &lower_left.x1,
                                                      &lower_left.y1,
                                                      &lower_left.x2,
                                                      &lower_left.y2);
          is_intersecting |= SDL_IntersectRectAndLine(&obsticle_rect,
                                                      &lower_right.x1,
                                                      &lower_right.y1,
                                                      &lower_right.x2,
                                                      &lower_right.y2);
          
          if (is_intersecting)
          {
            //// find the line with the shortest distance to its originating
            //// corner
            int current_distance;
            
            // upper left corner
            int index = 0;
            int shortest_distance = distance(min_x(collider_before_rect),
                                             min_y(collider_before_rect),
                                             upper_left.x1,
                                             upper_left.y1);
            
            // upper right corner
            current_distance = distance(max_x(collider_before_rect),
                                        min_y(collider_before_rect),
                                        upper_right.x1,
                                        upper_right.y1);
            if (current_distance < shortest_distance)
            {
              index = 1;
              shortest_distance = current_distance;
            }
            
            // lower left corner
            current_distance = distance(min_x(collider_before_rect),
                                        max_y(collider_before_rect),
                                        lower_left.x1,
                                        lower_left.y1);
            if (current_distance < shortest_distance)
            {
              index = 2;
              shortest_distance = current_distance;
            }
            
            // lower right corner
            current_distance = distance(max_x(collider_before_rect),
                                        max_y(collider_before_rect),
                                        lower_right.x1,
                                        lower_right.y1);
            if (current_distance < shortest_distance)
            {
              index = 3;
              shortest_distance = current_distance;
            }
            
            // update travel distance
            switch (index)
            {
              case 0:
                travel_distance.x = min_x(collider_before_rect)-upper_left.x1;
                travel_distance.y = min_y(collider_before_rect)-upper_left.y1;
                break;
              case 1:
                travel_distance.x = max_x(collider_before_rect)-upper_right.x1;
                travel_distance.y = min_y(collider_before_rect)-upper_right.y1;
                break;
              case 2:
                travel_distance.x = min_x(collider_before_rect)-lower_left.x1;
                travel_distance.y = max_y(collider_before_rect)-lower_left.y1;
                break;
              case 3:
                travel_distance.x = max_x(collider_before_rect)-lower_right.x1;
                travel_distance.y = max_y(collider_before_rect)-lower_right.y1;
                break;
            }
            
            // update velocity
            collider.changeVelocityTo(0, 0);
          }
        }
      }
    }
    else
    {
      // collider is inside obsticle (rare)
      result.push_back(&obsticle);
      
      if (is_responding)
      {
        //// find shortest distance to one of the obsticles edges
        int current_distance;
        
        // distance to upper edge
        int index = 0;
        int shortest_distance = min_y(obsticle_rect)-min_y(collider_before_rect);
        
        // distance to lower edge
        current_distance = max_y(obsticle_rect) - max_y(collider_before_rect);
        if (current_distance < shortest_distance)
        {
          index = 1;
          shortest_distance = current_distance;
        }
        
        // distance to left edge
        current_distance = min_x(obsticle_rect) - min_x(collider_before_rect);
        if (current_distance < shortest_distance)
        {
          index = 2;
          shortest_distance = current_distance;
        }
        
        // distance to right edge
        current_distance = max_x(obsticle_rect) - max_x(collider_before_rect);
        if (current_distance < shortest_distance)
        {
          index = 3;
          shortest_distance = current_distance;
        }
        
        // update travel distance
        switch (index)
        {
          case 0:
            travel_distance.x = 0;
            travel_distance.y = -(shortest_distance + collider_before_rect.h);
            break;
          case 1:
            travel_distance.x = 0;
            travel_distance.y = shortest_distance + collider_before_rect.h;
            break;
          case 2:
            travel_distance.x = -(shortest_distance + collider_before_rect.w);
            travel_distance.y = 0;
            break;
          case 3:
            travel_distance.x = shortest_distance + collider_before_rect.w;
            travel_distance.y = 0;
            break;
        }
        
        // update velocity
        collider.changeVelocityTo(0, 0);
      }
    }
  }
  
  for (auto child : obsticle.children())
  {
    _resolveCollisions(collider,
                       *child,
                       travel_distance,
                       collision_response,
                       result);
  }
}

void Core::resolveCollisions(Entity & collider,
                             Vector2 & travel_distance,
                             bool collision_response,
                             vector<Entity *> & result)
{
  _resolveCollisions(collider,
                     *root(),
                     travel_distance,
                     collision_response,
                     result);
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
  gravity({0.0, 9.82});
  dynamic(false);
  collision_detection(false);
  collision_response(false);
}

void PhysicsComponent::init(Entity * entity)
{
  Component::init(entity);
  
  _should_simulate = true;
  _out_of_view = true;
  _did_collide = false;
  
  auto did_start_animating = [this](Event) { _should_simulate = false; };
  auto did_stop_animating = [this](Event) { _should_simulate = true;  };
  
  auto animation = entity->animation();
  NotificationCenter::observe(did_start_animating,
                              DidStartAnimating,
                              animation);
  NotificationCenter::observe(did_stop_animating,
                              DidStopAnimating,
                              animation);
}

void PhysicsComponent::update(Core & core)
{
  
  // if simulating a dynamic entity, update its velocity
  Vector2 distance {};
  bool should_move = _should_simulate && dynamic();
  if (should_move)
  {
    const auto velocity = gravity() * core.delta_time() * pixels_per_meter;
    entity()->changeVelocityBy(velocity.x, velocity.y);
    distance = entity()->velocity() * core.delta_time();
  }
  
  // if enabled, perform collision detection and response
  collided_entities().clear();
  if (collision_detection())
  {
    core.resolveCollisions(*entity(),
                           distance,
                           should_move && collision_response(),
                           collided_entities());
    
    // notify observers if at least one collision ocurred
    if (collided_entities().size() > 0)
    {
      if (!_did_collide)
      {
        NotificationCenter::notify(DidCollide, *this);
        _did_collide = true;
      }
    }
    else _did_collide = false;
    
  }
  
  // if simulating a dynamic entity, update its position
  if (should_move)
  {
    entity()->moveBy(distance.x, distance.y);
  }
  
  // calculate if the entity has gone out of or into view
  Vector2 world_position;
  Dimension2 dimensions = entity()->dimensions();
  entity()->calculateWorldPosition(world_position);
  if (!_out_of_view &&
      (world_position.x + dimensions.x < 0 ||
       world_position.y + dimensions.y < 0 ||
       world_position.x >= core.view_dimensions().x ||
       world_position.y >= core.view_dimensions().y))
  {
    _out_of_view = true;
    NotificationCenter::notify(DidMoveOutOfView, *this);
  }
  else if (_out_of_view &&
           world_position.x + dimensions.x >= 0 &&
           world_position.y + dimensions.y >= 0 &&
           world_position.x < core.view_dimensions().x &&
           world_position.y < core.view_dimensions().y)
  {
    _out_of_view = false;
    NotificationCenter::notify(DidMoveIntoView, *this);
  }
}
