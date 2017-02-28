//
//  Board.hpp
//  Arcade Game Engine
//

#pragma once

#include "core.hpp"

const Dimension2 BOARD_DIMENSIONS { 224, 176 };


// MARK: Events
const Event DidClearBoard("DidClearBoard");
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
  
  Block(string id, int order, int x, int y);
  void init(Core * core);
  void touch();
};

/**
 *  Defines a board.
 */
class Board
  : public Entity
{
  bool _did_die;
  int _sum;
public:
  Board(string id);
  void init(Core * core);
  void reset();
};
