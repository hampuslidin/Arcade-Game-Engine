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
  
  sprites().clear();
  sprites().reserve(54);
  for (auto i = 0; i < 9; i++)
  {
    for (auto j = 0; j < 6; j++)
    {
      string filename = "textures/block_" + to_string(i) + "_" + to_string(j) +
      ".png";
      sprites().push_back(Sprite::createSprite(entity->core()->renderer(),
                                               filename.c_str()));
    }
  }
  
  resizeTo(32, 32);
}

void BlockGraphicsComponent::reset()
{
  current_sprite(sprites()[0]);
}

void BlockGraphicsComponent::changeColor(int base_i, int detail_i)
{
  _base_i = base_i;
  _detail_i = detail_i;
  current_sprite(sprites()[base_i*6+detail_i]);
}

void BlockGraphicsComponent::changeBaseColor(int index)
{
  changeColor(index, _detail_i);
}

void BlockGraphicsComponent::changeDetailColor(int index)
{
  changeColor(_base_i, index);
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
    block_graphics->changeDetailColor(5);
  }
}
