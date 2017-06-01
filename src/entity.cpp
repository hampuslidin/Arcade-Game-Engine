//
//  entity.cpp
//  Game Engine
//

#include "core.hpp"

// MARK: -
// MARK: Properties
const Core * Entity::core() const                { return _core; }
const Entity * Entity::parent() const            { return _parent; }
const vector<Entity*> & Entity::children() const { return _children; }

InputComponent * Entity::input() const         { return _input; }
AnimationComponent * Entity::animation() const { return _animation; }
ColliderComponent * Entity::collider() const   { return _collider; }
RigidBodyComponent * Entity::rigidBody() const { return _rigidBody; }
AudioComponent * Entity::audio() const         { return _audio; }
GraphicsComponent * Entity::graphics() const   { return _graphics; }
ParticleSystemComponent * Entity::particleSystem() const
{
  return _particleSystem;
}

const mat4 & Entity::previousWorldTransform() const
{
  return _previousWorldTransform;
}
const vec3 & Entity::localPosition() const    { return _localPosition; }
const quat & Entity::localOrientation() const { return _localOrientation; }
const vec3 & Entity::localScale() const       { return _localScale; }
const vec3 & Entity::velocity() const         { return _velocity; }
const vec3 & Entity::force() const            { return _force; }

bool Entity::enabled() const            { return _enabled; }
const EntityType & Entity::type() const { return _type; }

// MARK: Member functions
Entity::Entity()
  : _core(nullptr)
  , _parent(nullptr)
  , _children()
  , _input(nullptr)
  , _animation(nullptr)
  , _collider(nullptr)
  , _rigidBody(nullptr)
  , _audio(nullptr)
  , _graphics(nullptr)
  , _particleSystem(nullptr)
  , _localPosition(0.0f)
  , _localOrientation(1.0f, 0.0f, 0.0f, 0.0f)
  , _localScale(1.0f, 1.0f, 1.0f)
  , _velocity(0.0f)
  , _force(0.0f)
  , _enabled(false)
  , _type(Default)
  , _transformInvalid(true)
{}

void Entity::init(Core * core)
{
  // initialize properties
  _core    = core;
  _enabled = true;
  
  // initialize components
  if (_input)          _input->init(this);
  if (_animation)      _animation->init(this);
  if (_collider)       _collider->init(this);
  if (_rigidBody)      _rigidBody->init(this);
  if (_audio)          _audio->init(this);
  if (_graphics)       _graphics->init(this);
  if (_particleSystem) _particleSystem->init(this);
  
  // initial frame
  nextFrame();
  
  // set up notifications for when the transform of the entity changes
  if (_parent)
  {
    auto eventHandler = [this](Event)
    {
      _transformInvalid = true;
      NotificationCenter::notify(DidUpdateTransform, *this);
    };
    NotificationCenter::observe(eventHandler, DidUpdateTransform, _parent);
  }
  
  // recurse initialization on children
  for (auto child : _children) child->init(core);
}

void Entity::reset()
{
  if (_input)          _input->reset();
  if (_animation)      _animation->reset();
  if (_collider)       _collider->reset();
  if (_rigidBody)      _rigidBody->reset();
  if (_audio)          _audio->reset();
  if (_graphics)       _graphics->reset();
  if (_particleSystem) _particleSystem->reset();
  
  for (auto child : _children) child->reset();
}

void Entity::destroy()
{
  for (auto child : _children)
  {
    child->destroy();
  }
  _children.clear();
  
  if (_input)          delete _input;
  if (_animation)      delete _animation;
  if (_collider)       delete _collider;
  if (_rigidBody)      delete _rigidBody;
  if (_audio)          delete _audio;
  if (_graphics)       delete _graphics;
  if (_particleSystem) delete _particleSystem;
}

void Entity::nextFrame()
{
  _previousWorldTransform = worldTransform();
}

void Entity::addChild(Entity * child)
{
  _children.push_back(child);
  child->_parent = this;
}

Entity * Entity::findChild(string id)
{
  for (auto child : _children)
  {
    if (child->id().compare(id) == 0)
      return child;
    auto possible_find = child->findChild(id);
    if (possible_find)
      return possible_find;
  }
  return nullptr;
}

void Entity::removeChild(string id)
{
  for (int i = 0; i < _children.size(); i++)
  {
    auto child = _children[i];
    if (child->id() == id)
    {
      child->_parent = nullptr;
      _children.erase(_children.begin()+i);
    }
  }
}

void Entity::attachInputComponent(InputComponent * input)
{
  _input = input;
}

void Entity::attachAnimationComponent(AnimationComponent * animation)
{
  _animation = animation;
}

void Entity::attachRigidBodyComponent(RigidBodyComponent * rigidBody)
{
  _rigidBody = rigidBody;
}

void Entity::attachColliderComponent(ColliderComponent * collider)
{
  _collider = collider;
}

void Entity::attachAudioComponent(AudioComponent * audio)
{
  _audio = audio;
}

void Entity::attachGraphicsComponent(GraphicsComponent * graphics)
{
  _graphics = graphics;
}

void Entity::attachParticleSystemComponent
  (ParticleSystemComponent * particleSystem)
{
  _particleSystem = particleSystem;
}

mat4 Entity::localTransform() const
{
  return localTranslation() * localRotation();
}

mat4 Entity::localTranslation() const
{
  return glm::translate(_localPosition);
}

mat4 Entity::localRotation() const
{
  return glm::toMat4(_localOrientation);
}

vec3 Entity::localUp() const
{
  return glm::rotate(_localOrientation, Core::WORLD_UP);
}

vec3 Entity::localDown() const
{
  return glm::rotate(_localOrientation, Core::WORLD_DOWN);
}

vec3 Entity::localLeft() const
{
  return glm::rotate(_localOrientation, Core::WORLD_LEFT);
}

vec3 Entity::localRight() const
{
  return glm::rotate(_localOrientation, Core::WORLD_RIGHT);
}

vec3 Entity::localForward() const
{
  return glm::rotate(_localOrientation, Core::WORLD_FORWARD);
}

vec3 Entity::localBackward() const
{
  return glm::rotate(_localOrientation, Core::WORLD_BACKWARD);
}

const vec3 & Entity::worldPosition() const
{
  if (_transformInvalid) _updateTransform();
  return _worldPosition;
}

const quat & Entity::worldOrientation() const
{
  if (_transformInvalid) _updateTransform();
  return _worldOrientation;
}

const vec3 & Entity::worldScale() const
{
  if (_transformInvalid) _updateTransform();
  return _worldScale;
}

mat4 Entity::worldTransform() const
{
  return worldTranslation() * worldRotation() * worldScaling();
}

mat4 Entity::worldTranslation() const
{
  return glm::translate(worldPosition());
}

mat4 Entity::worldRotation() const
{
  return glm::toMat4(worldOrientation());
}

mat4 Entity::worldScaling() const
{
  return glm::scale(worldScale());
}

void Entity::translate(const vec3 & d)
{
  _localPosition += d;
  _transformInvalid = true;
  NotificationCenter::notify(DidUpdateTransform, *this);
}

void Entity::rotate(float angle, const vec3 & axis)
{
  quat newQuat = glm::angleAxis(angle, axis) * _localOrientation;
  _localOrientation = glm::normalize(newQuat);
  _transformInvalid = true;
  NotificationCenter::notify(DidUpdateTransform, *this);
}

void Entity::scale(float s)
{
  scale({s, s, s});
}

void Entity::scale(const vec3 & s)
{
  _localScale *= s;
  _transformInvalid = true;
  NotificationCenter::notify(DidUpdateTransform, *this);
}

void Entity::accelerate(const vec3 & v)
{
  _velocity += v;
}

void Entity::applyForce(const vec3 & f)
{
  _force += f;
}

void Entity::reposition(const vec3 & p)
{
  _localPosition.x = 0.0f;
  _localPosition.y = 0.0f;
  _localPosition.z = 0.0f;
  translate(p);
}

void Entity::repositionX(float x)
{
  _localPosition.x = 0.0f;
  translate({x, 0.0f, 0.0f});
}

void Entity::repositionY(float y)
{
  _localPosition.y = 0.0f;
  translate({0.0f, y, 0.0f});
}

void Entity::repositionZ(float z)
{
  _localPosition.z = 0.0f;
  translate({0.0f, 0.0f, z});
}

void Entity::reorient(const vec3 & o)
{
  quat qX = glm::angleAxis(o.x, Core::WORLD_RIGHT);
  quat qY = glm::angleAxis(o.y, Core::WORLD_UP);
  quat qZ = glm::angleAxis(o.z, Core::WORLD_BACKWARD);
  _localOrientation = qZ * qY * qX;
  _transformInvalid = true;
  NotificationCenter::notify(DidUpdateTransform, *this);
}

void Entity::resetPitch(float pitch)
{
  vec3 eulerAngles = glm::eulerAngles(_localOrientation);
  reorient({pitch, eulerAngles.y, eulerAngles.z});
}

void Entity::resetYaw(float yaw)
{
  vec3 eulerAngles = glm::eulerAngles(_localOrientation);
  reorient({eulerAngles.x, yaw, eulerAngles.z});
}

void Entity::resetRoll(float roll)
{
  vec3 eulerAngles = glm::eulerAngles(_localOrientation);
  reorient({eulerAngles.x, eulerAngles.y, roll});
}

void Entity::rescale(const vec3 & s)
{
  _localScale.x = 1.0f;
  _localScale.y = 1.0f;
  _localScale.z = 1.0f;
  scale(s);
}

void Entity::rescaleX(float x)
{
  _localScale.x = 1.0f;
  scale({x, 1.0f, 1.0f});
}

void Entity::rescaleY(float y)
{
  _localScale.y = 1.0f;
  translate({1.0f, y, 1.0f});
}

void Entity::rescaleZ(float z)
{
  _localScale.z = 1.0f;
  translate({1.0f, 1.0f, z});
}

void Entity::resetVelocity(const vec3 & v)
{
  _velocity.x = 0.0f;
  _velocity.y = 0.0f;
  _velocity.z = 0.0f;
  accelerate(v);
}

void Entity::resetVelocityX(float vx)
{
  _velocity.x = 0.0f;
  accelerate({vx, 0.0f, 0.0f});
}

void Entity::resetVelocityY(float vy)
{
  _velocity.y = 0.0f;
  accelerate({0.0f, vy, 0.0f});
}

void Entity::resetVelocityZ(float vz)
{
  _velocity.z = 0.0f;
  accelerate({0.0f, 0.0f, vz});
}

void Entity::resetForce(const vec3 & f)
{
  _force.x = 0.0f;
  _force.y = 0.0f;
  _force.z = 0.0f;
  applyForce(f);
}

void Entity::resetForceX(float fx)
{
  _force.x = 0.0f;
  applyForce({fx, 0.0f, 0.0f});
}

void Entity::resetForceY(float fy)
{
  _force.y = 0.0f;
  applyForce({0.0f, fy, 0.0f});
}

void Entity::resetForceZ(float fz)
{
  _force.z = 0.0f;
  applyForce({0.0f, 0.0f, fz});
}

void Entity::type(const EntityType & newType)
{
  _type = newType;
}

bool Entity::operator ==(Entity & entity)
{
  return id().compare(entity.id()) == 0;
}

bool Entity::operator !=(Entity & entity)
{
  return id().compare(entity.id()) != 0;
}

// MARK: Private
void Entity::_updateTransform() const
{
  if (_parent)
  {
    _worldPosition     = _parent->worldPosition() + _localPosition;
    _worldOrientation  = _parent->worldOrientation() * _localOrientation;
    _worldScale        = _parent->worldScale() * _localScale;
  }
  else
  {
    _worldPosition    = _localPosition;
    _worldOrientation = _localOrientation;
    _worldScale       = _localScale;
  }
  _transformInvalid = false;
}
