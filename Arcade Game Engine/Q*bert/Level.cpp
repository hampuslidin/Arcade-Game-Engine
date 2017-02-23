//
//  Level.cpp
//  Arcade Game Engine
//

#include "Level.hpp"
#include "Player.hpp"
#include "Enemy.hpp"
#include "Board.hpp"
#include "HUD.hpp"

//
// MARK: - Level
//
Level::Level(string id)
  : Entity(id, nullptr, nullptr, nullptr, nullptr)
{
  addChild(new Board("board"));
  addChild(new Player("player"));
  addChild(new HUD("hud"));
  
  string enemy_id;
  ControllerDirection default_direction;
  double speed;
  Vector2 gravity;
  Vector2 jump_up_end_point;
  Vector2 jump_down_end_point;
  Vector2 jump_left_end_point;
  Vector2 jump_right_end_point;
  
  // Ugg
  enemy_id = "ugg";
  default_direction = UP;
  speed = 0.7;
  gravity = {-1.417, -0.818};
  jump_up_end_point = {-16, -24};
  jump_down_end_point = {};
  jump_left_end_point = {-32, 0};
  jump_right_end_point = {};
  addChild(new Enemy(enemy_id, default_direction, speed, gravity,
                     jump_up_end_point, jump_down_end_point,
                     jump_left_end_point, jump_right_end_point));
}

void Level::reset()
{
  Entity::reset();
  
  game_over(false);
}
