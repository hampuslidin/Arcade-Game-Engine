//
//  types.hpp
//  Game Engine
//

#pragma once

#include <stdlib.h>

/**
 *  Defines a functor that enables classes to have class properties accessible
 *  by other types, while still preserving encapsulation.
 */

/* read and write */
template <typename PropertyType>
class prop {
  PropertyType _v;
public:
  PropertyType & operator()() { return _v; };
  void operator()(const PropertyType & v) { _v = v; };
  
};

/* read-only */
template <class entity, typename PropertyType>
class prop_r {
  friend entity;
  PropertyType _v;
public:
  PropertyType & operator()() { return _v; };
private:
  void operator()(const PropertyType & v) { _v = v; };
};


/**
 *  Defines an event for the notify-observe pattern.
 */
class Event
{
  const char * _id;
public:
  typedef int Parameter;
  prop_r<Event, Parameter> parameter;
  
  Event(const char * id);
  Event(const Event & event, const Parameter & parameter);
  bool operator==(const Event & event) const;
  bool operator<(const Event & event) const;
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
