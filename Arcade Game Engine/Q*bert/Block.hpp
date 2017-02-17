//
//  Block.hpp
//  Arcade Game Engine
//

#pragma once

#include "core.hpp"

/**
 *  Defines the block physics.
 */
class BlockPhysicsComponent : public PhysicsComponent
{
public:
  BlockPhysicsComponent();
};

/**
 *  Defines the block graphics.
 */
class BlockGraphicsComponent : public GraphicsComponent
{
public:
  enum BlockColor { ORANGE, BLUE, GREEN };
  
  void init(Entity * entity);
  void reset();
  void changeBaseColor(BlockColor block_color);
  void changeDetailColor(BlockColor block_color);
};

/**
 *  Defines a block.
 */
class Block : public Entity
{
public:
  Block(string id, int x, int y);
  void toggle(string id);
};
