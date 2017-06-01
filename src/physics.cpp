//
//  physics.cpp
//  Game Engine
//

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
const box & SphereColliderComponent::staticAxisAlignedBoundingBox() const
{
  return _staticAxisAlignedBoundingBox;
}
const box & SphereColliderComponent::dynamicAxisAlignedBoundingBox() const
{
  return _dynamicAxisAlignedBoundingBox;
}
float SphereColliderComponent::radius() const { return _radius; }

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
  vec3 & oldMin      = _staticAxisAlignedBoundingBox.min;
  vec3 & oldMax      = _staticAxisAlignedBoundingBox.max;
  vec3 newMin        = worldPosition - _radius;
  vec3 newMax        = worldPosition + _radius;
  
  _dynamicAxisAlignedBoundingBox.min = glm::min(oldMin, newMin);
  _dynamicAxisAlignedBoundingBox.max = glm::max(oldMax, newMax);
  oldMin = newMin;
  oldMax = newMax;
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
    const float t = core.deltaTime();
    const vec3 a = _gravity + entity()->force()/_mass;
    vec3 v = entity()->velocity() + a*t;
    float absoluteVelocity = glm::length(v);
    if (absoluteVelocity > _thermalVelocity)
      v *= _thermalVelocity / absoluteVelocity;
    entity()->resetVelocity(v);
    entity()->translate(v*float(core.deltaTime()));
    entity()->resetForce();
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
