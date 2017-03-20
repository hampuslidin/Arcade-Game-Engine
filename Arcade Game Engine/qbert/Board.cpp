//
//  Board.cpp
//  Arcade Game Engine
//

#include "Board.hpp"
#include "HUD.hpp"


//
// MARK: - BlockPhysicsComponent
//

// MARK: Member functions

BlockPhysicsComponent::BlockPhysicsComponent()
  : PhysicsComponent()
{
  collision_bounds({10, 8, 12, 12});
}


//
// MARK: - BlockGraphicsComponent
//

// MARK: Member functions

void BlockGraphicsComponent::init(Entity * entity)
{
  GraphicsComponent::init(entity);
  
  _base_i = 0;
  _detail_i = 0;
  
  resizeTo(32, 32);
}

void BlockGraphicsComponent::reset()
{
  GraphicsComponent::reset();
  
  if (((Block*)entity())->state() == Block::NOT_SET)
  {
    _base_i   = 0;
    _detail_i = 0;
  }
  
  changeColor(_base_i, _detail_i);
}

void BlockGraphicsComponent::changeColor(int base_i, int detail_i)
{
  _base_i = base_i;
  _detail_i = detail_i;
  string id = "block" + to_string(_base_i*6) + to_string(_detail_i);
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

Block::Block(string id, int order, int x, int y)
  : Entity(id, order)
  , state(NOT_SET)
{
  addPhysics(new BlockPhysicsComponent());
  addGraphics(new BlockGraphicsComponent());
  
  moveTo(x, y);
}

void Block::init(Core * core)
{
  Entity::init(core);
  
  auto reset = [this](Event)
  {
    state(NOT_SET);
  };
  
  NotificationCenter::observe(reset, DidClearBoard);
  NotificationCenter::observe(reset, DidDie);
}

void Block::touch()
{
  if (state() == NOT_SET)
  {
    auto block_graphics = (BlockGraphicsComponent*)graphics();
    block_graphics->changeDetailColor(1);
    state(FULL_SET);
    NotificationCenter::notify(Event(DidSetBlock, FULL_SET), *this);
  }
}

//
// MARK: - Board
//

Board::Board(string id)
  : Entity(id, 10)
{
  for (auto n = 0; n < 7; n++)
  {
    for (auto m = 0; m < n + 1; m++)
    {
      string id = "block" + to_string(n+1) + to_string(m+1);
      addChild(new Block(id,
                         order() + 10*n,
                         (int)BOARD_DIMENSIONS.x/2 - 16*(n+1) + 32*m,
                         24*n));
    }
  }
}

void Board::init(Core * core)
{
  Entity::init(core);
  
  _did_die = false;
  _sum = 28;
  
  SpriteCollection & sprites = SpriteCollection::main();
  for (auto i = 0; i < 9; i++)
  {
    for (auto j = 0; j < 6; j++)
    {
      string id = "block" + to_string(i) + to_string(j);
      string filename = "textures/" + id + ".png";
      sprites.create(id, filename.c_str());
    }
  }
  
  auto did_set_block = [this, core](Event event)
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
      NotificationCenter::notify(DidClearBoard, *this);
      _sum = 28;
      core->pause();
      core->reset(1.0);
    }
  };
  auto did_die = [this](Event) { _did_die = true; };
  
  NotificationCenter::observe(did_set_block, DidSetBlock);
  NotificationCenter::observe(did_die, DidDie);
  
  const Dimension2 view_dimensions = core->view_dimensions();
  moveTo((view_dimensions.x-BOARD_DIMENSIONS.x)/2,
         view_dimensions.y-BOARD_DIMENSIONS.y-16);
}

void Board::reset()
{
  Entity::reset();
  
  if (_did_die)
  {
    _did_die = false;
    _sum = 28;
  }
}
