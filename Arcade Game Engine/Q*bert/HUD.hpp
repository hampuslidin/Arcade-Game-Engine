//
//  HUD.hpp
//  Arcade Game Engine
//

#pragma once

#include "core.hpp"
#include "Level.hpp"
#include "Board.hpp"
#include "Player.hpp"


// MARK: Events

const Event DidDie("DidDie");


//
// MARK: - PlayerTextGraphicsComponent
//

class PlayerTextGraphicsComponent
  : public GraphicsComponent
{
  const double _duration = 0.5;
  double _start_time;
  int _current_sprite_index;
public:
  void init(Entity * entity);
  void reset();
  void update(Core & core);
};


//
// MARK: - PlayerText
//

class PlayerText : public Entity
{
public:
  PlayerText(string id);
  void init(Core * core);
};


//
// MARK: - ScoreDigitGraphicsComponent
//

class ScoreDigitGraphicsComponent
  : public GraphicsComponent
{
public:
  void init(Entity * entity);
  void update(Core & core);
};


//
// MARK: - ScoreDigit
//

class ScoreDigit
  : public Entity
{
  bool _did_die;
public:
  prop<int> digit;
  
  ScoreDigit(string id, int x, int y);
  void init(Core * core);
  void reset();
};


//
// MARK: - Score
//

class Score
: public Entity
{
  bool _did_die;
  Level * _level;
  
  void update_digits();
public:
  prop_r<Score, int> score;
  
  Score(string id);
  void init(Core * core);
  void reset();
};


//
// MARK: - LifeGraphicsComponent
//

class LifeGraphicsComponent
  : public GraphicsComponent
{
public:
  void init(Entity * entity);
  void update(Core & core);
};


//
// MARK: - Life
//

class Life
  : public Entity
{
  bool _did_die;
public:
  prop<bool> visible;
  
  Life(string id, int x, int y);
  void init(Core * core);
  void reset();
};


//
// MARK: - HUD
//

class HUD : public Entity
{
  int _lives = 3;
  bool _did_die;
public:
  HUD(string id);
  void init(Core * core);
  void reset();
};
