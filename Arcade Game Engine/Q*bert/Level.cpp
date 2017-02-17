//
//  Level.cpp
//  Arcade Game Engine
//

#include "Level.hpp"
#include "Player.hpp"
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
}

void Level::reset()
{
  Entity::reset();
  
  game_over(false);
}
