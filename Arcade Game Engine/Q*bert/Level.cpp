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
  : Entity(id, -1)
{
  addChild(new Board("board"));
  addChild(new Player("player"));
  
  string enemy_id;
  int order;
  ControllerDirection default_direction;
  pair<int, int> default_board_position;
  double speed;
  Vector2 gravity;
  Vector2 end_points[4];
  pair<int, int> board_position_changes[4];
  
  // Ugg
  enemy_id = "ugg";
  order = 91; // TODO: should work from 81 and up, but it doesn't for some reason
  default_direction = UP;
  default_board_position = {6, 6};
  speed = 0.7;
  gravity = {-1.417, -0.818};
  end_points[UP]    = {-16, -24};
  end_points[DOWN]  = {};
  end_points[LEFT]  = {-32, 0};
  end_points[RIGHT] = {};
  board_position_changes[UP]   = {-1, -1};
  board_position_changes[LEFT] = {0,  -1};
  addChild(new Enemy(enemy_id,
                     order,
                     default_direction,
                     default_board_position,
                     speed,
                     gravity,
                     end_points,
                     board_position_changes));
  
  addChild(new HUD("hud"));
}

void Level::reset()
{
  Entity::reset();
  
  game_over(false);
}
