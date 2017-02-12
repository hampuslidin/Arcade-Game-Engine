//
//  types.h
//  Game Engine
//

#pragma once

/**
 *  Defines an event for the notify-observe pattern.
 */
typedef const char * Event;


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
  float x, y;
};

inline Vector2 operator+(Vector2 l, Vector2 r)  { return {l.x+r.x, l.y+r.y}; }
inline Vector2 operator+(Vector2 l, float c)    { return {l.x+c, l.y+c}; }
inline Vector2 operator+(float c, Vector2 r)    { return {r.x+c, r.y+c}; }
inline Vector2 operator+=(Vector2 l, Vector2 r) { return l = l + r; }
inline Vector2 operator+=(Vector2 l, float c)   { return l = l + c; }

inline Vector2 operator-(Vector2 v)             { return {-v.x, -v.y}; }
inline Vector2 operator-(Vector2 l, Vector2 r)  { return l + (-r); }
inline Vector2 operator-(Vector2 l, float c)    { return l + (-c); }
inline Vector2 operator-(float c, Vector2 r)    { return c + (-r); }
inline Vector2 operator-=(Vector2 l, Vector2 r) { return l += -r; }
inline Vector2 operator-=(Vector2 l, float c)   { return l += -c; }

inline Vector2 operator*(Vector2 l, Vector2 r)  { return {l.x*r.x, l.y*r.y}; }
inline Vector2 operator*(Vector2 l, float c)    { return {l.x*c, l.y*c}; }
inline Vector2 operator*(float c, Vector2 r)    { return {r.x*c, r.y*c}; }
inline Vector2 operator*=(Vector2 l, Vector2 r) { return l = l * r; }
inline Vector2 operator*=(Vector2 l, float c)   { return l = l * c; }

inline Vector2 recip(Vector2 v)                 { return {1.f/v.x, 1.f/v.y}; }
inline Vector2 operator/(Vector2 l, Vector2 r)  { return l * recip(r); }
inline Vector2 operator/(Vector2 l, float c)    { return l * (1.f/c); }
inline Vector2 operator/(float c, Vector2 r)    { return c * recip(r); }
inline Vector2 operator/=(Vector2 l, Vector2 r) { return l = l / r; }
inline Vector2 operator/=(Vector2 l, float c)   { return l = l / c; }


/**
 *  Defines two dimensions by width and height.
 */
struct Dimension2
{
  float w, h;
};


/**
 *  Defines a rectangle specified by its position and dimensions.
 */
struct Rectangle
{
  Vector2 pos;
  Dimension2 dim;
};

inline float min_x(Rectangle r) { return r.pos.x; }
inline float min_y(Rectangle r) { return r.pos.y; }
inline float max_x(Rectangle r) { return r.pos.x + r.dim.w; }
inline float max_y(Rectangle r) { return r.pos.y + r.dim.h; }


/**
 *  Defines a functor that enables classes to have class properties accessible
 *  by other types, while still preserving encapsulation.
 */

/* read and write */
template <typename T>
struct prop {
  T _v;
public:
  inline T & operator()() { return _v; };
  inline void operator()(T v) { _v = v; };
  
};

/* read-only */
template <class C, typename T>
struct prop_r {
  friend C;
  T _v;
public:
  inline T & operator()() { return _v; };
private:
  inline void operator()(T v) { _v = v; };
};
