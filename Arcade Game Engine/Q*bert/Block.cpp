//
//  Block.cpp
//  Arcade Game Engine
//

#include "Block.hpp"

/********************************
 * BlockPhysicsComponent
 ********************************/
BlockPhysicsComponent::BlockPhysicsComponent()
  : PhysicsComponent()
{
  collision_bounds({0, 0, 32, 32});
  dynamic(false);
}

/********************************
 * BlockGraphicsComponent
 ********************************/
void BlockGraphicsComponent::init(Entity * entity)
{
  GraphicsComponent::init(entity);
  
  initSprites(*entity->core()->renderer(), {"textures/block_o_b_1.png"});
  
  resizeTo(32, 32);
}

/********************************
 * Block
 ********************************/
Block::Block(int x, int y)
  : Entity(nullptr,
           nullptr,
           new BlockPhysicsComponent(),
           new BlockGraphicsComponent())
{
  moveTo(x, y);
}
