//
//  HUD.cpp
//  Arcade Game Engine
//

#include "HUD.hpp"

// MARK: Helper functions

int number_of_digits(int n)
{
  if (n != 0)
  {
    int num_digits = 0;
    int tmp = n;
    while (tmp > 0)
    {
      tmp /= 10;
      num_digits += 1;
    }
    return num_digits;
  }
  return 1;
}


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
  string id = "player_1_text_" + to_string(_current_sprite_index);
  current_sprite(SpriteCollection::main().retrieve(id));
  
  GraphicsComponent::update(core);
}

//
// MARK: - PlayerText
//

PlayerText::PlayerText(string id)
  : Entity(id, 100)
{
  addGraphics(new PlayerTextGraphicsComponent());
}

void PlayerText::init(Core * core)
{
  Entity::init(core);
  
  SpriteCollection & sprites = SpriteCollection::main();
  for (auto i = 0; i < 6; i++)
  {
    string id = "player_1_text_" + to_string(i);
    string filename = "textures/" + id + ".png";
    sprites.create(id, filename.c_str());
  }
  
  moveTo(8, 0);
}

//
// MARK: - Score
//

// MARK: Member functions

Score::Score(string id)
  : Entity(id, 100)
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
    string filename = "textures/score_digit_orange_" + to_string(n) + ".png";
    sprites.create(id, filename.c_str());
  }
  
  auto did_set_block = [this](Event event)
  {
    switch (event.parameter())
    {
      case Block::HALF_SET:
        score(score()+15);
        break;
      case Block::FULL_SET:
        score(score()+25);
        break;
    }
    update_digits();
  };
  
  NotificationCenter::observe(did_set_block, DidSetBlock);
  
  _level = (Level*)(core->root());
  moveTo(10, 12);
}

void Score::reset()
{
  Entity::reset();
  
  score(0);
  update_digits();
}

// MARK: Private member functions

void Score::update_digits()
{
  int tmp = score();
  for (auto i = number_of_digits(score())-1; i >= 0; i--, tmp /= 10)
  {
    ((ScoreDigit*)children()[i])->digit(tmp % 10);
  }
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
  : Entity(id, 100)
{
  addGraphics(new ScoreDigitGraphicsComponent());
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
  : Entity(id, 100)
{
  addChild(new PlayerText("player_text"));
  addChild(new Score("score"));
}

void HUD::init(Core * core)
{
  Entity::init(core);
  
  moveTo(8, 8);
}
