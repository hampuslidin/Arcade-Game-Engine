//
//  Board.cpp
//  Arcade Game Engine
//

#include "Board.hpp"

#define BOARD_DIMENSIONS ((Dimension2){224, 176})


//
// MARK: - BlockPhysicsComponent
//

// MARK: Member functions

BlockPhysicsComponent::BlockPhysicsComponent()
  : PhysicsComponent()
{
  collision_bounds({15, 6, 2, 2});
}


//
// MARK: - BlockGraphicsComponent
//

// MARK: Member functions

void BlockGraphicsComponent::init(Entity * entity)
{
  GraphicsComponent::init(entity);
  
  resizeTo(32, 32);
}

void BlockGraphicsComponent::reset()
{
  GraphicsComponent::reset();
  
  current_sprite(SpriteCollection::main().retrieve("block_0_0"));
}

void BlockGraphicsComponent::changeColor(int base_i, int detail_i)
{
  _base_i = base_i;
  _detail_i = detail_i;
  string id = "block_" + to_string(_base_i*6) + "_" + to_string(_detail_i);
  current_sprite(SpriteCollection::main().retrieve(id));
}

void BlockGraphicsComponent::changeBaseColor(int index)
{
  changeColor(index, _detail_i);
}

void BlockGraphicsComponent::changeDetailColor(int index)
{
  changeColor(_base_i, index);
}


//
// MARK: - Block
//

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
    block_graphics->changeDetailColor(1);
  }
}

//
// MARK: - Board
//

Board::Board(string id)
  : Entity(id, nullptr, nullptr, nullptr, nullptr)
{
  for (auto n = 0; n < 7; n++)
  {
    for (auto m = 0; m < n + 1; m++)
    {
      string id = "block" + to_string(n+1) + to_string(m+1);
      addChild(new Block(id, BOARD_DIMENSIONS.x/2 - 16*(n+1) + 32*m, 24*n));
    }
  }
}

void Board::init(Core * core)
{
  Entity::init(core);
  
  SpriteCollection & sprites = SpriteCollection::main();
  for (auto i = 0; i < 9; i++)
  {
    for (auto j = 0; j < 6; j++)
    {
      string id = "block_" + to_string(i) + "_" + to_string(j);
      string filename = "textures/" + id + ".png";
      sprites.create(id, filename.c_str());
    }
  }
  
  const Dimension2 view_dimensions = core->view_dimensions();
  moveTo((view_dimensions.x-BOARD_DIMENSIONS.x)/2,
         view_dimensions.y-BOARD_DIMENSIONS.y-16);
}
