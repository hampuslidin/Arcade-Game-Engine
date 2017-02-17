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
  collision_bounds({15, 6, 2, 2});
}

/********************************
 * BlockGraphicsComponent
 ********************************/
void BlockGraphicsComponent::init(Entity * entity)
{
  GraphicsComponent::init(entity);
  
  vector<const char *> filenames
  {
    "textures/block_o_b.png",
    "textures/block_o_g.png"
  };
  initSprites(*entity->core()->renderer(), filenames);
  
  resizeTo(32, 32);
}

void BlockGraphicsComponent::reset()
{
  current_sprite(sprites()[0]);
}

void BlockGraphicsComponent::changeBaseColor(BlockColor block_color)
{
  // TODO: Implement.
}

void BlockGraphicsComponent::changeDetailColor(BlockColor block_color)
{
  // TODO: Make generic.
  current_sprite(sprites()[1]);
}

/********************************
 * Block
 ********************************/
Block::Block(string id, int x, int y)
  : Entity(id,
           nullptr,
           nullptr,
           new BlockPhysicsComponent(),
           new BlockGraphicsComponent())
{
  moveTo(x, y);
}

void Block::toggle(string id)
{
  if (id.compare("player") == 0)
  {
    BlockGraphicsComponent * block_graphics =
      dynamic_cast<BlockGraphicsComponent*>(graphics());
    block_graphics->changeDetailColor(BlockGraphicsComponent::GREEN);
  }
}
