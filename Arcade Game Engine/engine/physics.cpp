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
                        vec3 & travelDistance,
                        bool collisionResponse,
                        vector<Entity*> & result)
{
  if (collider.pPhysics() && obsticle.pPhysics() && &collider != &obsticle)
  {
    Rectangle colliderCB = collider.pPhysics()->collisionBounds();
    Rectangle obsticleCB = obsticle.pPhysics()->collisionBounds();
    vec3 colPos = collider.worldPosition();
    vec3 obsPos = obsticle.worldPosition();
    
    SDL_Rect colliderBeforeRect
    {
      (int)(colPos.x + colliderCB.pos.x),
      (int)(colPos.y + colliderCB.pos.y),
      (int)colliderCB.dim.x,
      (int)colliderCB.dim.y
    };
    
    SDL_Rect obsticleRect
    {
      (int)(obsPos.x + obsticleCB.pos.x),
      (int)(obsPos.y + obsticleCB.pos.y),
      (int)obsticleCB.dim.x,
      (int)obsticleCB.dim.y
    };
    
    // second condition is a hack for enemies not colliding away from each other
    bool isResponding = collisionResponse && !obsticle.pPhysics()->dynamic();
    
    // check so that the collider is not already colliding with obsticle
    SDL_Rect intersectionRect;
    if (!SDL_IntersectRect(&colliderBeforeRect,
                           &obsticleRect,
                           &intersectionRect))
    {
      // collider is outside of obsticle
      SDL_Rect colliderAfterRect
      {
        (int)(colPos.x + colliderCB.pos.x + travelDistance.x),
        (int)(colPos.y + colliderCB.pos.y + travelDistance.y),
        (int)colliderCB.dim.x,
        (int)colliderCB.dim.y
      };
      
      // the bounding box for the colliders before and after traviling the
      // distance
      SDL_Rect largeRect;
      largeRect.x = (int)std::min(min_x(colliderBeforeRect),
                                  min_x(colliderAfterRect));
      largeRect.y = (int)std::min(min_y(colliderBeforeRect),
                                  min_y(colliderAfterRect));
      largeRect.w = (int)std::max(max_x(colliderBeforeRect),
                                  max_x(colliderAfterRect)) - largeRect.x;
      largeRect.h = (int)std::max(max_y(colliderBeforeRect),
                                  max_y(colliderAfterRect)) - largeRect.y;
      
      if (SDL_IntersectRect(&largeRect, &obsticleRect, &intersectionRect))
      {
        // collider will collide with obsticle between this frame and the next
        result.push_back(&obsticle);
        
        if (isResponding)
        {
          typedef struct { int x1, y1, x2, y2; } Line;
          
          auto distance = [](int x1, int y1, int x2, int y2)
          {
            double dx = x2-x1;
            double dy = y2-y1;
            return sqrt(dx*dx+dy*dy);
          };
          
          Line upperLeft, upperRight, lowerLeft, lowerRight;
          
          upperLeft.x1  = (int)min_x(colliderBeforeRect);
          upperLeft.y1  = (int)min_y(colliderBeforeRect);
          upperLeft.x2  = (int)min_x(colliderAfterRect);
          upperLeft.y2  = (int)min_y(colliderAfterRect);
          
          upperRight.x1 = (int)max_x(colliderBeforeRect);
          upperRight.y1 = (int)min_y(colliderBeforeRect);
          upperRight.x2 = (int)max_x(colliderAfterRect);
          upperRight.y2 = (int)min_y(colliderAfterRect);
          
          lowerLeft.x1  = (int)min_x(colliderBeforeRect);
          lowerLeft.y1  = (int)max_y(colliderBeforeRect);
          lowerLeft.x2  = (int)min_x(colliderAfterRect);
          lowerLeft.y2  = (int)max_y(colliderAfterRect);
          
          lowerRight.x1 = (int)max_x(colliderBeforeRect);
          lowerRight.y1 = (int)max_y(colliderBeforeRect);
          lowerRight.x2 = (int)max_x(colliderAfterRect);
          lowerRight.y2 = (int)max_y(colliderAfterRect);
          
          int intersections = 0;
          intersections += SDL_IntersectRectAndLine(&obsticleRect,
                                                    &upperLeft.x1,
                                                    &upperLeft.y1,
                                                    &upperLeft.x2,
                                                    &upperLeft.y2);
          intersections += SDL_IntersectRectAndLine(&obsticleRect,
                                                    &upperRight.x1,
                                                    &upperRight.y1,
                                                    &upperRight.x2,
                                                    &upperRight.y2);
          intersections += SDL_IntersectRectAndLine(&obsticleRect,
                                                    &lowerLeft.x1,
                                                    &lowerLeft.y1,
                                                    &lowerLeft.x2,
                                                    &lowerLeft.y2);
          intersections += SDL_IntersectRectAndLine(&obsticleRect,
                                                    &lowerRight.x1,
                                                    &lowerRight.y1,
                                                    &lowerRight.x2,
                                                    &lowerRight.y2);
          
          if (intersections > 0)
          {
            //// find the line with the shortest distance to its originating
            //// corner
            int currentDistance;
            
            // upper left corner
            int index = 0;
            int shortestDistance = (int)distance((int)min_x(colliderBeforeRect),
                                                 (int)min_y(colliderBeforeRect),
                                                 upperLeft.x1,
                                                 upperLeft.y1);
            
            // upper right corner
            currentDistance = (int)distance((int)max_x(colliderBeforeRect),
                                            (int)min_y(colliderBeforeRect),
                                            upperRight.x1,
                                            upperRight.y1);
            if (currentDistance < shortestDistance)
            {
              index = 1;
              shortestDistance = currentDistance;
            }
            
            // lower left corner
            currentDistance = (int)distance((int)min_x(colliderBeforeRect),
                                            (int)max_y(colliderBeforeRect),
                                            lowerLeft.x1,
                                            lowerLeft.y1);
            if (currentDistance < shortestDistance)
            {
              index = 2;
              shortestDistance = currentDistance;
            }
            
            // lower right corner
            currentDistance = (int)distance((int)max_x(colliderBeforeRect),
                                            (int)max_y(colliderBeforeRect),
                                            lowerRight.x1,
                                            lowerRight.y1);
            if (currentDistance < shortestDistance)
            {
              index = 3;
              shortestDistance = currentDistance;
            }
            
            // update travel distance
            switch (index)
            {
              case 0:
                travelDistance.x = min_x(colliderBeforeRect)-upperLeft.x1;
                travelDistance.y = min_y(colliderBeforeRect)-upperLeft.y1;
                break;
              case 1:
                travelDistance.x = max_x(colliderBeforeRect)-upperRight.x1;
                travelDistance.y = min_y(colliderBeforeRect)-upperRight.y1;
                break;
              case 2:
                travelDistance.x = min_x(colliderBeforeRect)-lowerLeft.x1;
                travelDistance.y = max_y(colliderBeforeRect)-lowerLeft.y1;
                break;
              case 3:
                travelDistance.x = max_x(colliderBeforeRect)-lowerRight.x1;
                travelDistance.y = max_y(colliderBeforeRect)-lowerRight.y1;
                break;
            }
            
            // update velocity
            collider.pVelocity() = vec3(0.0f);
          }
        }
      }
    }
    else
    {
      // collider is inside obsticle (rare)
      result.push_back(&obsticle);
      
      if (isResponding)
      {
        //// find shortest distance to one of the obsticles edges
        int current_distance;
        
        // distance to upper edge
        int index = 0;
        int shortest_distance = 
          (int)min_y(obsticleRect) - (int)min_y(colliderBeforeRect);
        
        // distance to lower edge
        current_distance = (int)max_y(obsticleRect) - (int)max_y(colliderBeforeRect);
        if (current_distance < shortest_distance)
        {
          index = 1;
          shortest_distance = current_distance;
        }
        
        // distance to left edge
        current_distance = (int)min_x(obsticleRect) - (int)min_x(colliderBeforeRect);
        if (current_distance < shortest_distance)
        {
          index = 2;
          shortest_distance = current_distance;
        }
        
        // distance to right edge
        current_distance = (int)max_x(obsticleRect) - (int)max_x(colliderBeforeRect);
        if (current_distance < shortest_distance)
        {
          index = 3;
          shortest_distance = current_distance;
        }
        
        // update travel distance
        switch (index)
        {
          case 0:
            travelDistance.x = 0;
            travelDistance.y = -(shortest_distance + colliderBeforeRect.h);
            break;
          case 1:
            travelDistance.x = 0;
            travelDistance.y = shortest_distance + colliderBeforeRect.h;
            break;
          case 2:
            travelDistance.x = -(shortest_distance + colliderBeforeRect.w);
            travelDistance.y = 0;
            break;
          case 3:
            travelDistance.x = shortest_distance + colliderBeforeRect.w;
            travelDistance.y = 0;
            break;
        }
        
        // update velocity
        collider.pVelocity = vec3(0.0f);
      }
    }
  }
  
  for (auto child : obsticle.children())
  {
    _resolveCollisions(collider,
                       *child,
                       travelDistance,
                       collisionResponse,
                       result);
  }
}

void Core::resolveCollisions(Entity & collider,
                             vec3 & travelDistance,
                             bool collisionResponse,
                             vector<Entity *> & result)
{
  _resolveCollisions(collider,
                     root(),
                     travelDistance,
                     collisionResponse,
                     result);
}


//
// MARK: - PhysicsComponent
//

// MARK: Property functions

string PhysicsComponent::trait() { return "physics"; }

// MARK: Member functions

PhysicsComponent::PhysicsComponent()
  : collisionBounds({0, 0, 16, 16})
  , gravity({0.0f, -9.82f, 0.0f})
  , dynamic(false)
  , collisionDetection(false)
  , collisionResponse(false)
{}

void PhysicsComponent::init(Entity * entity)
{
  Component::init(entity);
  
  _shouldSimulate = true;
  _outOfView = true;
  _didCollide = false;
  
  auto eventHandler = [this](Event event)
  {
    _shouldSimulate = event == DidStopAnimating;
  };
  
  auto animation = entity->pAnimation();
  NotificationCenter::observe(eventHandler,
                              DidStartAnimating,
                              animation);
  NotificationCenter::observe(eventHandler,
                              DidStopAnimating,
                              animation);
}

void PhysicsComponent::update(Core & core)
{
  
  // if simulating a dynamic entity, update its velocity
  vec3 distance(0.0f);
  bool shouldMove = _shouldSimulate && dynamic();
  if (shouldMove)
  {
    const auto velocity = gravity() * (float)(core.deltaTime()*pixelsPerMeter);
    entity()->pVelocity() += velocity;
    distance = entity()->pVelocity() * (float)core.deltaTime();
  }
  
  // if enabled, perform collision detection and response
  collidedEntities().clear();
  if (collisionDetection())
  {
    core.resolveCollisions(*entity(),
                           distance,
                           shouldMove && collisionResponse(),
                           collidedEntities());
    
    // notify observers if at least one collision ocurred
    if (collidedEntities().size() > 0)
    {
      if (!_didCollide)
      {
        NotificationCenter::notify(DidCollide, *this);
        _didCollide = true;
      }
    }
    else _didCollide = false;
    
  }
  
  // if simulating a dynamic entity, update its position
  if (shouldMove)
  {
    entity()->translate(distance.x, distance.y, distance.z);
  }
  
  // calculate if the entity has gone out of or into view
  vec3 worldPosition = entity()->worldPosition();
  auto windowDimensions = core.viewDimensions();
  Dimension2 dimensions = entity()->dimensions();
  if (!_outOfView &&
      (worldPosition.x + dimensions.x < 0 ||
       worldPosition.y + dimensions.y < 0 ||
       worldPosition.x >= windowDimensions.x ||
       worldPosition.y >= windowDimensions.y))
  {
    _outOfView = true;
    NotificationCenter::notify(DidMoveOutOfView, *this);
  }
  else if (_outOfView &&
           worldPosition.x + dimensions.x >= 0 &&
           worldPosition.y + dimensions.y >= 0 &&
           worldPosition.x < windowDimensions.x &&
           worldPosition.y < windowDimensions.y)
  {
    _outOfView = false;
    NotificationCenter::notify(DidMoveIntoView, *this);
  }
}
