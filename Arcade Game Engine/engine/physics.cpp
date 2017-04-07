//
//  physics.cpp
//  Game Engine
//

#include <algorithm>
#include "core.hpp"


// MARK: Properties
const vec3 & ColliderComponent::origin() const { return _origin; }

// MARK: Member functions
ColliderComponent::ColliderComponent(const vec3 & origin)
  : _origin(origin)
{}

void ColliderComponent::reposition(const vec3 & origin)
{
  _origin = origin;
}

// MARK: -
// MARK: Properties
const box & SphereColliderComponent::axisAlignedBoundingBox() const
{
  return _axisAlignedBoundingBox;
}

// MARK: Member functions
SphereColliderComponent::SphereColliderComponent(float radius)
  : ColliderComponent()
  , _radius(radius)
{}

SphereColliderComponent::SphereColliderComponent(const vec3 & origin,
                                                 float radius)
  : ColliderComponent(origin)
  , _radius(radius)
{}

void SphereColliderComponent::update(const Core & core)
{
  vec3 worldPosition = origin() + entity()->worldPosition();
  _axisAlignedBoundingBox.min = worldPosition - _radius;
  _axisAlignedBoundingBox.max = worldPosition + _radius;
}

bool SphereColliderComponent::collide(const ColliderComponent & obsticle,
                                      const vec3 & colliderPosition,
                                      const vec3 & obsticlePosition) const
{
  string obsticleTrait = obsticle.trait();
  if (obsticleTrait.compare(trait()) == 0)
  {
    auto sphereObsticle = (const SphereColliderComponent&)obsticle;
    vec3 intersection;
    if (Core::SphereIntersect(colliderPosition,
                              obsticlePosition,
                              _radius,
                              sphereObsticle._radius,
                              intersection))
    {
      printf("Collided!\n");
      return true;
    }
  }
  return false;
}

void SphereColliderComponent::resize(float radius)
{
  _radius = radius;
}

string SphereColliderComponent::trait() const
{
  return "SphereCollider";
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
    const vec3 v = a*t*float(Core::UNITS_PER_METER);
    entity()->accelerate(v.x, v.y, v.z);
    
    // adjust for thermal velocity
    vec3 u = entity()->velocity();
    if (glm::length(u) > _thermalVelocity)
    {
      u = glm::normalize(u) * _thermalVelocity;
      entity()->resetVelocity(u.x, u.y, u.z);
    }
    
    // update position
    const vec3 d = entity()->velocity()*t;
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
