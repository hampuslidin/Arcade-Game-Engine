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

void Block::reset()
{
  Entity::reset();
  state(NOT_SET);
}

void Block::toggle_state()
{
  if (state() == NOT_SET)
  {
    auto block_graphics = (BlockGraphicsComponent*)graphics();
    block_graphics->changeDetailColor(1);
    state(FULL_SET);
    NotificationCenter::main().notify(Event(DidSetBlock, FULL_SET));
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
  
  auto did_set_block = [this, core](Event event, vector<GameObject*> *)
  {
    switch (event.parameter())
    {
      case Block::NOT_SET:
        _sum += 1;
        break;
      case Block::FULL_SET:
        _sum -= 1;
        break;
    }
    if (_sum == 0)
    {
      NotificationCenter::main().notify(DidClearBoard);
      core->createTimer(1, [core]()
      {
        core->reset();
      });
    }
  };
  
  NotificationCenter::main().observe(DidSetBlock, did_set_block);
  
  const Dimension2 view_dimensions = core->view_dimensions();
  moveTo((view_dimensions.x-BOARD_DIMENSIONS.x)/2,
         view_dimensions.y-BOARD_DIMENSIONS.y-16);
}

void Board::reset()
{
  Entity::reset();
  
  _sum = 28;
}
