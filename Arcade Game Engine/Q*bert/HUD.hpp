//
//  HUD.hpp
//  Arcade Game Engine
//

#pragma once

#include "core.hpp"
#include "Level.hpp"

/**
 *  Defines a heads-up display.
 */
class HUD : public Entity
{
public:
  HUD(string id);
  void init(Core * core);
};

/**
 *  Defines player text graphics.
 */
class PlayerTextGraphicsComponent : public GraphicsComponent
{
  const double _duration = 0.5;
  double _start_time;
  int _current_sprite_index;
public:
  void init(Entity * entity);
  void reset();
  void update(Core & core);
};

/**
 *  Defines player text.
 */
class PlayerText : public Entity
{
public:
  PlayerText(string id);
  void init(Core * core);
};

/**
 *  Defines score.
 */
class Score
  : public Entity
{
  Level * _level;
public:
  prop_r<Score, int> score;
  
  Score(string id);
  void init(Core * core);
  void reset();
};

/**
 *  Defines score graphics.
 */
class ScoreDigitGraphicsComponent
  : public GraphicsComponent
{
public:
  void init(Entity * entity);
  void reset();
};

/**
 *  Defines a score digit.
 */
class ScoreDigit : public Entity
{
public:
  prop_r<ScoreDigit, int> digit;
  
  ScoreDigit(string id, int x, int y);
  void reset();
};
