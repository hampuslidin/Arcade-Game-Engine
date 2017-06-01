//
//  animation.cpp
//  Arcade Game Engine
//

#include "core.hpp"


// MARK: Properties
bool AnimationComponent::animating() const           { return _animating; }
const vec3 & AnimationComponent::endVelocity() const { return _endVelocity; }

// MARK: Member functions
void AnimationComponent::reset()
{
  _animating = false;
  _updateVelocity = false;
}

void AnimationComponent::animate(const Core & core)
{
  if (_animating)
  {
    const double dt = _duration / (_currentCurve.size() - 1);
    const double elapsed  = core.effectiveElapsedTime() - _startTime;
    if (elapsed < _duration)
    {
      const int i = (int)floor(elapsed / dt);
      const double t = fmod(elapsed, dt) / dt;
      const double t2 = t*t;
      const double t3 = t2*t;
      const double two_t3 = 2*t3;
      const double three_t2 = 3*t2;
      const float cp0 = two_t3 - three_t2 + 1;
      const float cm0 = t3 - 2*t2 + t;
      const float cm1 = t3 - t2;
      const float cp1 = three_t2 - two_t3;
      const auto s0 = _currentCurve[i];
      const auto s1 = _currentCurve[i+1];
      const vec3 p = s0.first*cp0 + s0.second*cm0 +
      s1.first*cp1 + s1.second*cm1;
      entity()->reposition(_startPosition + p);
    }
    else
    {
      auto lastHalfSpline = _currentCurve.back();
      entity()->reposition(_startPosition + lastHalfSpline.first);
      if (_updateVelocity)
        entity()->resetVelocity(lastHalfSpline.second/float(_duration));
      _animating = false;
      NotificationCenter::notify(DidStopAnimating, *this);
    }
  }
}

void AnimationComponent::addSegment(string id, vec3 position, vec3 velocity)
{
  _curves[id].push_back({position, velocity});
}

void AnimationComponent::removeCurve(string id)
{
  _curves.erase(id);
}

void AnimationComponent::performAnimation(string id,
                                          double duration,
                                          bool updateVelocity)
{
  if (!_animating && _curves.find(id) != _curves.end())
  {
    _animating = true;
    _currentCurve = _curves[id];
    _startPosition = entity()->localPosition();
    _startTime = entity()->core()->effectiveElapsedTime();
    _duration = duration;
    _updateVelocity = updateVelocity;
    NotificationCenter::notify(DidStartAnimating, *this);
  }
}

string AnimationComponent::trait() const { return "Animation"; }
