//
//  animation.cpp
//  Arcade Game Engine
//

#include "core.hpp"
#include <fstream>

// MARK: Helper functions
inline double transform_from_range(double value,
                                   double from_low_bound,
                                   double from_high_bound,
                                   double to_low_bound,
                                   double to_high_bound)
{
  return value/(from_high_bound-from_low_bound)*(to_high_bound-to_low_bound);
}


//
// MARK: - AnimationComponent
//

// MARK: Property functions

string AnimationComponent::trait() { return "animation"; }

// MARK: Member functions

void AnimationComponent::reset()
{
  animating(false);
  _update_velocity = false;
}

void AnimationComponent::addSegment(string id, Vector2 point, Vector2 velocity)
{
  _curves[id].push_back({point, velocity});
}

void AnimationComponent::removeCurve(string id)
{
  _curves.erase(id);
}

void AnimationComponent::performAnimation(string id,
                                          double duration,
                                          bool update_velocity)
{
  if (!animating() && _curves.find(id) != _curves.end())
  {
    animating(true);
    _current_curve = _curves[id];
    _start_position = entity()->local_position();
    _start_time = entity()->core()->elapsedTime();
    _duration = duration;
    _update_velocity = update_velocity;
    NotificationCenter::notify(DidStartAnimating, *this);
  }
}

void AnimationComponent::update(Core & world)
{
  if (animating())
  {
    const double dt = _duration / (_current_curve.size() - 1);
    const double elapsed  = world.elapsedTime() - _start_time;
    if (elapsed < _duration)
    {
      const int i = floor(elapsed / dt);
      const double t = fmod(elapsed, dt) / dt;
      const double t2 = t*t;
      const double t3 = t2*t;
      const double cp0 = 2*t3 - 3*t2 + 1;
      const double cm0 = t3 - 2*t2 + t;
      const double cm1 = t3 - t2;
      const double cp1 = -2*t3 + 3*t2;
      const auto s0 = _current_curve[i];
      const auto s1 = _current_curve[i+1];
      const Vector2 p = s0.first*cp0 + s0.second*cm0 +
                        s1.first*cp1 + s1.second*cm1;
      
      entity()->moveTo(_start_position.x + p.x, _start_position.y + p.y);
    }
    else
    {
      auto last_half_segment = _current_curve.back();
      entity()->moveTo(_start_position.x + last_half_segment.first.x,
                       _start_position.y + last_half_segment.first.y);
      if (_update_velocity)
      {
        entity()->changeVelocityTo(last_half_segment.second.x/_duration,
                                   last_half_segment.second.y/_duration);
      }
      animating(false);
      NotificationCenter::notify(DidStopAnimating, *this);
    }
  }
}
