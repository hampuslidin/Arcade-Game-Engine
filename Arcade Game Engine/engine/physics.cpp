//
//  physics.cpp
//  Game Engine
//

#include <algorithm>
#include "core.hpp"


// MARK: Member functions
void Core::resolveCollisions(Entity & collider) const
{
  vec3 colliderPosition = collider.worldPosition();
  for (auto obsticle : _entities)
  {
    if (collider.collider() && obsticle.collider() && collider != obsticle)
    {
      vec3 obsticlePosition = obsticle.worldPosition();
      collider.collider()->collide(*obsticle.collider(),
                                   colliderPosition,
                                   obsticlePosition);
    }
  }
}

bool Core::AABBIntersect(box & a, box & b, box & intersection)
{
  intersection = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}};
  
  for (int i = 0; i < 3; i++)
  {
    const float aMin_i = a.min[i];
    const float aMax_i = a.max[i];
    const float bMin_i = b.min[i];
    const float bMax_i = b.max[i];
    
    if (aMin_i > bMax_i || bMin_i > aMax_i)
      return false;
    
    intersection.min[i] = std::max(aMin_i, bMin_i);
    intersection.max[i] = std::min(aMax_i, bMax_i);
  }
  
  return true;
  
//
//  SDL_Rect colliderBeforeRect
//  {
//    (int)(colPos.x + colliderCB.pos.x),
//    (int)(colPos.y + colliderCB.pos.y),
//    (int)colliderCB.dim.x,
//    (int)colliderCB.dim.y
//  };
//  
//  SDL_Rect obsticleRect
//  {
//    (int)(obsPos.x + obsticleCB.pos.x),
//    (int)(obsPos.y + obsticleCB.pos.y),
//    (int)obsticleCB.dim.x,
//    (int)obsticleCB.dim.y
//  };
//  
//  // second condition is a hack for enemies not colliding away from each other
//  bool isResponding = collisionResponse && !obsticle.pRigidBody()->dynamic();
//  
//  // check so that the collider is not already colliding with obsticle
//  SDL_Rect intersectionRect;
//  if (!SDL_IntersectRect(&colliderBeforeRect,
//                         &obsticleRect,
//                         &intersectionRect))
//  {
//    // collider is outside of obsticle
//    SDL_Rect colliderAfterRect
//    {
//      (int)(colPos.x + colliderCB.pos.x + travelDistance.x),
//      (int)(colPos.y + colliderCB.pos.y + travelDistance.y),
//      (int)colliderCB.dim.x,
//      (int)colliderCB.dim.y
//    };
//    
//    // the bounding box for the colliders before and after traviling the
//    // distance
//    SDL_Rect largeRect;
//    largeRect.x = (int)std::min(min_x(colliderBeforeRect),
//                                min_x(colliderAfterRect));
//    largeRect.y = (int)std::min(min_y(colliderBeforeRect),
//                                min_y(colliderAfterRect));
//    largeRect.w = (int)std::max(max_x(colliderBeforeRect),
//                                max_x(colliderAfterRect)) - largeRect.x;
//    largeRect.h = (int)std::max(max_y(colliderBeforeRect),
//                                max_y(colliderAfterRect)) - largeRect.y;
//    
//    if (SDL_IntersectRect(&largeRect, &obsticleRect, &intersectionRect))
//    {
//      // collider will collide with obsticle between this frame and the next
//      result.push_back(&obsticle);
//      
//      if (isResponding)
//      {
//        typedef struct { int x1, y1, x2, y2; } Line;
//        
//        auto distance = [](int x1, int y1, int x2, int y2)
//        {
//          double dx = x2-x1;
//          double dy = y2-y1;
//          return sqrt(dx*dx+dy*dy);
//        };
//        
//        Line upperLeft, upperRight, lowerLeft, lowerRight;
//        
//        upperLeft.x1  = (int)min_x(colliderBeforeRect);
//        upperLeft.y1  = (int)min_y(colliderBeforeRect);
//        upperLeft.x2  = (int)min_x(colliderAfterRect);
//        upperLeft.y2  = (int)min_y(colliderAfterRect);
//        
//        upperRight.x1 = (int)max_x(colliderBeforeRect);
//        upperRight.y1 = (int)min_y(colliderBeforeRect);
//        upperRight.x2 = (int)max_x(colliderAfterRect);
//        upperRight.y2 = (int)min_y(colliderAfterRect);
//        
//        lowerLeft.x1  = (int)min_x(colliderBeforeRect);
//        lowerLeft.y1  = (int)max_y(colliderBeforeRect);
//        lowerLeft.x2  = (int)min_x(colliderAfterRect);
//        lowerLeft.y2  = (int)max_y(colliderAfterRect);
//        
//        lowerRight.x1 = (int)max_x(colliderBeforeRect);
//        lowerRight.y1 = (int)max_y(colliderBeforeRect);
//        lowerRight.x2 = (int)max_x(colliderAfterRect);
//        lowerRight.y2 = (int)max_y(colliderAfterRect);
//        
//        int intersections = 0;
//        intersections += SDL_IntersectRectAndLine(&obsticleRect,
//                                                  &upperLeft.x1,
//                                                  &upperLeft.y1,
//                                                  &upperLeft.x2,
//                                                  &upperLeft.y2);
//        intersections += SDL_IntersectRectAndLine(&obsticleRect,
//                                                  &upperRight.x1,
//                                                  &upperRight.y1,
//                                                  &upperRight.x2,
//                                                  &upperRight.y2);
//        intersections += SDL_IntersectRectAndLine(&obsticleRect,
//                                                  &lowerLeft.x1,
//                                                  &lowerLeft.y1,
//                                                  &lowerLeft.x2,
//                                                  &lowerLeft.y2);
//        intersections += SDL_IntersectRectAndLine(&obsticleRect,
//                                                  &lowerRight.x1,
//                                                  &lowerRight.y1,
//                                                  &lowerRight.x2,
//                                                  &lowerRight.y2);
//        
//        if (intersections > 0)
//        {
//          //// find the line with the shortest distance to its originating
//          //// corner
//          int currentDistance;
//          
//          // upper left corner
//          int index = 0;
//          int shortestDistance = (int)distance((int)min_x(colliderBeforeRect),
//                                               (int)min_y(colliderBeforeRect),
//                                               upperLeft.x1,
//                                               upperLeft.y1);
//          
//          // upper right corner
//          currentDistance = (int)distance((int)max_x(colliderBeforeRect),
//                                          (int)min_y(colliderBeforeRect),
//                                          upperRight.x1,
//                                          upperRight.y1);
//          if (currentDistance < shortestDistance)
//          {
//            index = 1;
//            shortestDistance = currentDistance;
//          }
//          
//          // lower left corner
//          currentDistance = (int)distance((int)min_x(colliderBeforeRect),
//                                          (int)max_y(colliderBeforeRect),
//                                          lowerLeft.x1,
//                                          lowerLeft.y1);
//          if (currentDistance < shortestDistance)
//          {
//            index = 2;
//            shortestDistance = currentDistance;
//          }
//          
//          // lower right corner
//          currentDistance = (int)distance((int)max_x(colliderBeforeRect),
//                                          (int)max_y(colliderBeforeRect),
//                                          lowerRight.x1,
//                                          lowerRight.y1);
//          if (currentDistance < shortestDistance)
//          {
//            index = 3;
//            shortestDistance = currentDistance;
//          }
//          
//          // update travel distance
//          switch (index)
//          {
//            case 0:
//              travelDistance.x = min_x(colliderBeforeRect)-upperLeft.x1;
//              travelDistance.y = min_y(colliderBeforeRect)-upperLeft.y1;
//              break;
//            case 1:
//              travelDistance.x = max_x(colliderBeforeRect)-upperRight.x1;
//              travelDistance.y = min_y(colliderBeforeRect)-upperRight.y1;
//              break;
//            case 2:
//              travelDistance.x = min_x(colliderBeforeRect)-lowerLeft.x1;
//              travelDistance.y = max_y(colliderBeforeRect)-lowerLeft.y1;
//              break;
//            case 3:
//              travelDistance.x = max_x(colliderBeforeRect)-lowerRight.x1;
//              travelDistance.y = max_y(colliderBeforeRect)-lowerRight.y1;
//              break;
//          }
//          
//          // update velocity
//          collider.pVelocity() = vec3(0.0f);
//        }
//      }
//    }
//  }
//  else
//  {
//    // collider is inside obsticle (rare)
//    result.push_back(&obsticle);
//    
//    if (isResponding)
//    {
//      //// find shortest distance to one of the obsticles edges
//      int current_distance;
//      
//      // distance to upper edge
//      int index = 0;
//      int shortest_distance =
//      (int)min_y(obsticleRect) - (int)min_y(colliderBeforeRect);
//      
//      // distance to lower edge
//      current_distance = (int)max_y(obsticleRect) - (int)max_y(colliderBeforeRect);
//      if (current_distance < shortest_distance)
//      {
//        index = 1;
//        shortest_distance = current_distance;
//      }
//      
//      // distance to left edge
//      current_distance = (int)min_x(obsticleRect) - (int)min_x(colliderBeforeRect);
//      if (current_distance < shortest_distance)
//      {
//        index = 2;
//        shortest_distance = current_distance;
//      }
//      
//      // distance to right edge
//      current_distance = (int)max_x(obsticleRect) - (int)max_x(colliderBeforeRect);
//      if (current_distance < shortest_distance)
//      {
//        index = 3;
//        shortest_distance = current_distance;
//      }
//      
//      // update travel distance
//      switch (index)
//      {
//        case 0:
//          travelDistance.x = 0;
//          travelDistance.y = -(shortest_distance + colliderBeforeRect.h);
//          break;
//        case 1:
//          travelDistance.x = 0;
//          travelDistance.y = shortest_distance + colliderBeforeRect.h;
//          break;
//        case 2:
//          travelDistance.x = -(shortest_distance + colliderBeforeRect.w);
//          travelDistance.y = 0;
//          break;
//        case 3:
//          travelDistance.x = shortest_distance + colliderBeforeRect.w;
//          travelDistance.y = 0;
//          break;
//      }
//      
//      // update velocity
//      collider.pVelocity = vec3(0.0f);
//    }
//  }
}


// MARK: -
// MARK: Member functions
void ColliderComponent::update(const Core & core)
{
  core.resolveCollisions(*entity());
}


// MARK: -
// MARK: Member functions
void AABBColliderComponent::collide(const ColliderComponent & obsticle,
                                    const vec3 & colliderPosition,
                                    const vec3 & obsticlePosition) const
{
  string obsticleTrait = obsticle.trait();
  
  if (obsticleTrait.compare("AABBCollider") == 0)
  {
    auto obsticleAABB = (AABBColliderComponent&)obsticle;
    box colliderBox
    {
      colliderPosition + _collisionBox.min,
      colliderPosition + _collisionBox.max
    };
    box obsticleBox
    {
      obsticlePosition + obsticleAABB._collisionBox.min,
      obsticlePosition + obsticleAABB._collisionBox.max
    };
    box intersection;
    if (Core::AABBIntersect(colliderBox, obsticleBox, intersection))
      printf("Collided!\n");
  }
}

void AABBColliderComponent::resizeCollisionBox(const vec3 & min,
                                               const vec3 & max)
{
  _collisionBox.min = min;
  _collisionBox.min = max;
}

string AABBColliderComponent::trait() const
{
  return "AABBCollider";
}


// MARK: -
// MARK: Properties
float RigidBodyComponent::mass() const            { return _mass; }
float RigidBodyComponent::thermalVelocity() const { return _thermalVelocity; }
const vec3 & RigidBodyComponent::gravity() const  { return _gravity; }
bool RigidBodyComponent::kinematic() const        { return _kinematic; }

// MARK: Member functions
RigidBodyComponent::RigidBodyComponent()
  : _mass(1.0f)
  , _thermalVelocity(INFINITY)
  , _gravity(0.0f, -9.82f, 0.0f)
  , _kinematic(false)
{}

void RigidBodyComponent::init(Entity * entity)
{
  Component::init(entity);
  
  _shouldSimulate = true;
  
  auto eventHandler = [this](Event event)
  {
    _shouldSimulate = event == DidStopAnimating;
  };
  
  auto animation = entity->animation();
  NotificationCenter::observe(eventHandler,
                              DidStartAnimating,
                              animation);
  NotificationCenter::observe(eventHandler,
                              DidStopAnimating,
                              animation);
}

void RigidBodyComponent::update(const Core & core)
{
  if (_shouldSimulate && _kinematic)
  {
    // apply external forces to entity
    const float t = core.deltaTime();
    const vec3 a = _gravity + entity()->force()/_mass;
    const vec3 v = a*t;
    entity()->accelerate(v.x, v.y, v.z);
    
    // adjust for thermal velocity
    vec3 u = entity()->velocity();
    if (glm::length(u) > _thermalVelocity)
    {
      u = glm::normalize(u) * _thermalVelocity;
      entity()->resetVelocity(u.x, u.y, u.z);
    }
    
    // update position
    const vec3 d = entity()->velocity()*t*float(Core::UNITS_PER_METER);
    entity()->translate(d.x, d.y, d.z);
  }
}

void RigidBodyComponent::setMass(float m)
{
  _mass = m;
}

void RigidBodyComponent::setThermalVelocity(float v)
{
  _thermalVelocity = v;
}

void RigidBodyComponent::setGravity(float gx, float gy, float gz)
{
  _gravity.x = gx;
  _gravity.y = gy;
  _gravity.z = gz;
}

void RigidBodyComponent::setKinematic(bool enabled)
{
  _kinematic = enabled;
}

string RigidBodyComponent::trait() const
{
  return "RigidBody";
}
