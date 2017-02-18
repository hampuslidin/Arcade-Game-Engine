//
//  HUD.cpp
//  Arcade Game Engine
//

#include "HUD.hpp"


//
// MARK: - PlayerTextGraphicsComponent
//

// MARK: Member functions

void PlayerTextGraphicsComponent::init(Entity * entity)
{
  GraphicsComponent::init(entity);
  
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
  string id = "player_text_" + to_string(_current_sprite_index);
  current_sprite(SpriteCollection::main().retrieve(id));
  
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
  
  SpriteCollection & sprites = SpriteCollection::main();
  for (auto i = 0; i < 6; i++)
  {
    string id = "player_text_" + to_string(i);
    string filename = "textures/" + id + ".png";
    sprites.create(id, filename.c_str());
  }
  
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
    string id = "score_digit_" + to_string(n);
    addChild(new ScoreDigit(id, 8*n, 0));
  }
}

void Score::init(Core * core)
{
  Entity::init(core);
  
  SpriteCollection & sprites = SpriteCollection::main();
  for (auto n = 0; n < 10; n++)
  {
    string id = "score_digit_" + to_string(n);
    string filename = "textures/" + id + ".png";
    sprites.create(id, filename.c_str());
  }
  
  auto f = [this](Event, vector<GameObject*> *)
  {
    score(score()+25);
    int num_digits = 0;
    int tmp = score();
    while (tmp > 0)
    {
      tmp /= 10;
      num_digits += 1;
    }
    
    tmp = score();
    for (auto i = num_digits-1; i >= 0; i--, tmp /= 10)
    {
      ((ScoreDigit*)children()[i])->digit(tmp % 10);
    }
  };
  
  NotificationCenter::main().observe(DidCollide, f);
  
  _level = (Level*)(core->root());
  moveTo(8, 16);
}

void Score::reset()
{
  Entity::reset();
    
  score(0);
}

//
// MARK: - ScoreDigitGraphicsComponent
//

// MARK: Member functions

void ScoreDigitGraphicsComponent::init(Entity * entity)
{
  GraphicsComponent::init(entity);
  
  resizeTo(8, 16);
}

void ScoreDigitGraphicsComponent::update(Core & core)
{
  int digit = ((ScoreDigit*)entity())->digit();
  if (digit >= 0 && digit <= 9)
  {
    string id = "score_digit_" + to_string(digit);
    current_sprite(SpriteCollection::main().retrieve(id));
  }
  else
  {
    current_sprite(nullptr);
  }
    
  GraphicsComponent::update(core);
}

//
// MARK: - ScoreDigit
//

ScoreDigit::ScoreDigit(string id, int x, int y)
  : Entity(id, nullptr, nullptr, nullptr, new ScoreDigitGraphicsComponent())
{
  moveTo(x, y);
}

void ScoreDigit::init(Core * core)
{
  Entity::init(core);
}

void ScoreDigit::reset()
{
  Entity::reset();
  
  digit(-1);
}


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
