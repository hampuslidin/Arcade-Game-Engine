//
//  Level.cpp
//  Arcade Game Engine
//

#include "Level.hpp"
#include "Player.hpp"
#include "Ugg.hpp"
#include "Wrongway.hpp"
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
  addChild(new Ugg());
  addChild(new Wrongway());
  addChild(new HUD("hud"));
}

void Level::reset()
{
  Entity::reset();
  
  game_over(false);
}
