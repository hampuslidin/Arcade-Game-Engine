//
//  Board.cpp
//  Arcade Game Engine
//

#include "Board.hpp"

#define BOARD_DIMENSIONS ((Dimension2){224, 176})

/********************************
 * Board
 ********************************/
Board::Board()
  : Entity(nullptr, nullptr, nullptr, nullptr)
{
  for (auto n = 0; n < 7; n++)
  {
    for (auto m = 0; m < n + 1; m++)
    {
      string id = "block" + to_string(n+1) + to_string(m+1);
      addChild(new Block(BOARD_DIMENSIONS.x/2 - 16*(n+1) + 32*m, 24*n), id);
    }
  }
}

void Board::init(Core * core)
{
  const Dimension2 view_dimensions = core->view_dimensions();
  moveTo((view_dimensions.x-BOARD_DIMENSIONS.x)/2,
         view_dimensions.y-BOARD_DIMENSIONS.y-16);
  
  Entity::init(core);
}
