//
//  HUD.cpp
//  Arcade Game Engine
//

#include "HUD.hpp"

//
// MARK: - HUD
//

HUD::HUD(string id)
  : Entity(id, nullptr, nullptr, nullptr, nullptr)
{
  addChild(new PlayerText("player_text"));
  addChild(new Score("score"));
}

void HUD::init(Core * core)
{
  Entity::init(core);
  
  moveTo(8, 8);
}

//
// MARK: - PlayerTextGraphicsComponent
//

void PlayerTextGraphicsComponent::init(Entity * entity)
{
  GraphicsComponent::init(entity);
  
  sprites().reserve(6);
  for (auto i = 0; i < 6; i++)
  {
    string filename = "textures/player_text_" + to_string(i) + ".png";
    sprites().push_back(Sprite::createSprite(entity->core()->renderer(),
                                             filename.c_str()));
  }
  
  resizeTo(64, 11);
}

void PlayerTextGraphicsComponent::reset()
{
  GraphicsComponent::reset();
  
  _start_time = entity()->core()->elapsedTime();
  _current_sprite_index = 0;
}

void PlayerTextGraphicsComponent::update(Core & core)
{
  auto elapsed = core.elapsedTime() - _start_time;
  double cycles;
  modf(elapsed/_duration, &cycles);
  _start_time = _start_time + cycles * _duration;
  _current_sprite_index = floor(fmod(elapsed, _duration) / _duration * 6);
  current_sprite(sprites()[_current_sprite_index]);
  
  GraphicsComponent::update(core);
}

//
// MARK: - PlayerText
//

PlayerText::PlayerText(string id)
  : Entity(id, nullptr, nullptr, nullptr, new PlayerTextGraphicsComponent())
{}

void PlayerText::init(Core * core)
{
  Entity::init(core);
  
  moveTo(8, 0);
}

//
// MARK: - Score
//
Score::Score(string id)
  : Entity(id, nullptr, nullptr, nullptr, nullptr)
{
  for (auto n = 0; n < 10; n++)
  {
    string id = "textures/score_digit_" + to_string(n) + ".png";
    addChild(new ScoreDigit(id, 8*n, 0));
  }
}

void Score::init(Core * core)
{
  Entity::init(core);
  
  _level = (Level*)(core->root());
  moveTo(8, 16);
}

void Score::reset()
{
  Entity::reset();
    
  score(10000);
}

//
// MARK: - ScoreDigitGraphicsComponent
//

void ScoreDigitGraphicsComponent::init(Entity * entity)
{
  GraphicsComponent::init(entity);
  
  sprites().reserve(10);
  for (auto n = 0; n < 10; n++)
  {
    string filename = "textures/score_digit_" + to_string(n) + ".png";
    sprites().push_back(Sprite::createSprite(entity->core()->renderer(),
                                             filename.c_str()));
  }
  
  resizeTo(8, 16);
}

void ScoreDigitGraphicsComponent::reset()
{
  GraphicsComponent::reset();
  
  current_sprite(sprites()[((ScoreDigit*)entity())->digit()]);
}

//
// MARK: - ScoreDigit
//

ScoreDigit::ScoreDigit(string id, int x, int y)
  : Entity(id, nullptr, nullptr, nullptr, new ScoreDigitGraphicsComponent())
{
  moveTo(x, y);
}

void ScoreDigit::reset()
{
  digit(arc4random_uniform(10));
  
  Entity::reset();
}
