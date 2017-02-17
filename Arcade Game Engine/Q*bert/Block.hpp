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
class Block : public Entity
{
public:
  Block(string id, int x, int y);
  void toggle(string id);
};
