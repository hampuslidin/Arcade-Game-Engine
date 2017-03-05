//
//  types.hpp
//  Game Engine
//

#pragma once

#include <stdlib.h>
#include <string>

#ifdef __APPLE__
# include <SDL2/SDL.h>
#elif defined(_WIN32)
# include "SDL.h"
#endif

using namespace std;

/**
 *  Defines a functor that enables classes to have class properties accessible
 *  by other types, while still preserving encapsulation.
 */

/* read and write */
template <typename PropertyType>
class prop
{

public:
  prop()               : _v()  {};
  prop(PropertyType v) : _v(v) {};
  PropertyType & operator()() { return _v; };
  void operator()(const PropertyType & v) { _v = v; };

private:
  PropertyType _v;
  
};

/* read-only */
template <class Friend, typename PropertyType>
class prop_r
{

public:
  friend Friend;

  prop_r()               : _v()  {};
  prop_r(PropertyType v) : _v(v) {};
  PropertyType & operator()() { return _v; };

private:
  PropertyType _v;
  
  void operator()(const PropertyType & v) { _v = v; };
};

/**
 *  A wrapper class for when a certain values are optional.
 */
template <typename Value>
class maybe
{
  
public:
  static maybe<Value> just(Value & v) { return maybe(v); }
  static maybe<Value> nothing()       { return maybe();  }
  const Value * operator()() { return _just ? &_v : nullptr; }
  
private:
  bool _just;
  Value _v;
  
  maybe()        : _just(false)       {}
  maybe(Value v) : _v(v), _just(true) {}
};


/**
 *  Defines an event for the notify-observe pattern.
 */
class Event
{
  string _id;
public:
  typedef int Parameter;
  prop_r<Event, Parameter> parameter;
  
  string string_value();
  
  Event(string id);
  Event(Event event, Parameter parameter);
  bool operator==(Event event) const;
  bool operator<(Event event) const;
  
  void operator=(Event const &) = delete;
};


/**
 *  Defines a 24-bit RGB color.
 */
struct RGBColor
{
  uint8_t r, g, b;
};


/**
 *  Defines a 32-bit RGBA color.
 */
struct RGBAColor
{
  uint8_t r, g, b, a;
};


/**
 *  Defines a two-dimensional vector.
 */
struct Vector2
{
  double x, y;
};

inline Vector2 operator+ (Vector2   l, Vector2 r) { return {l.x+r.x, l.y+r.y}; }
inline Vector2 operator+ (Vector2   l, double  c) { return {l.x+c, l.y+c}; }
inline Vector2 operator+ (double    c, Vector2 r) { return {r.x+c, r.y+c}; }
inline Vector2 operator+=(Vector2 & l, Vector2 r) { return l = l + r; }
inline Vector2 operator+=(Vector2 & l, double  c) { return l = l + c; }

inline Vector2 operator- (Vector2   v)            { return {-v.x, -v.y}; }
inline Vector2 operator- (Vector2   l, Vector2 r) { return l + (-r); }
inline Vector2 operator- (Vector2   l, double  c) { return l + (-c); }
inline Vector2 operator- (double    c, Vector2 r) { return c + (-r); }
inline Vector2 operator-=(Vector2 & l, Vector2 r) { return l += -r; }
inline Vector2 operator-=(Vector2 & l, double  c) { return l += -c; }

inline Vector2 operator* (Vector2   l, Vector2 r) { return {l.x*r.x, l.y*r.y}; }
inline Vector2 operator* (Vector2   l, double  c) { return {l.x*c, l.y*c}; }
inline Vector2 operator* (double    c, Vector2 r) { return {r.x*c, r.y*c}; }
inline Vector2 operator*=(Vector2 & l, Vector2 r) { return l = l * r; }
inline Vector2 operator*=(Vector2 & l, double  c) { return l = l * c; }

inline Vector2 recip     (Vector2   v)            { return {1.f/v.x, 1.f/v.y}; }
inline Vector2 operator/ (Vector2   l, Vector2 r) { return l * recip(r); }
inline Vector2 operator/ (Vector2   l, double  c) { return l * (1.f/c); }
inline Vector2 operator/ (double    c, Vector2 r) { return c * recip(r); }
inline Vector2 operator/=(Vector2 & l, Vector2 r) { return l = l / r; }
inline Vector2 operator/=(Vector2 & l, double  c) { return l = l / c; }


/**
 *  Defines two dimensions by width and height.
 */
typedef Vector2 Dimension2;

/**
 *  Defines a rectangle specified by its position and dimensions.
 */
struct Rectangle
{
  Vector2 pos;
  Dimension2 dim;
};

inline double min_x(Rectangle r) { return r.pos.x; }
inline double min_y(Rectangle r) { return r.pos.y; }
inline double max_x(Rectangle r) { return r.pos.x + r.dim.x; }
inline double max_y(Rectangle r) { return r.pos.y + r.dim.y; }

inline double min_x(SDL_Rect r) { return r.x; }
inline double min_y(SDL_Rect r) { return r.y; }
inline double max_x(SDL_Rect r) { return r.x + r.w; }
inline double max_y(SDL_Rect r) { return r.y + r.h; }
