//
//  Board.hpp
//  Arcade Game Engine
//

#pragma once

#include "core.hpp"

const Event DidClearBoard("DidClearBoard");

// MARK: Events
const Event DidSetBlock("DidSetBlock");

/**
 *  Defines the block physics.
 */
class BlockPhysicsComponent
  : public PhysicsComponent
{
public:
  BlockPhysicsComponent();
};

/**
 *  Defines the block graphics.
 */
class BlockGraphicsComponent
  : public GraphicsComponent
{
  int _base_i;
  int _detail_i;
public:
  void init(Entity * entity);
  void reset();
  void changeColor(int base_i, int detail_i);
  void changeBaseColor(int index);
  void changeDetailColor(int index);
};

/**
 *  Defines a block.
 */
class Block
  : public Entity
{
public:
  enum State { NOT_SET, HALF_SET, FULL_SET };
  
  prop_r<Block, State> state;
  
  Block(string id, int x, int y);
  void reset();
  void toggle_state();
};

/**
 *  Defines a board.
 */
class Board
  : public Entity
{
  int _sum;
public:
  Board(string id);
  void init(Core * core);
  void reset();
};
