//
//  Board.hpp
//  Arcade Game Engine
//

#pragma once

#include "core.hpp"
#include "Block.hpp"

/**
 *  Defines a board.
 */
class Board : public Entity
{
public:
  Board(string id);
  void init(Core * core);
};
