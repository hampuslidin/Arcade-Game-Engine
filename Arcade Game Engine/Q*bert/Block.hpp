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
  void init(Entity * entity);
};

/**
 *  Defines a block.
 */
class Block : public Entity
{
public:
  Block(int x, int y);
};
