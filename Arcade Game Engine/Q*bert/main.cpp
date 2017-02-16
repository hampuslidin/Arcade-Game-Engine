//
//  main.cpp
//  Game Engine
//

#include "core.hpp"
#include "Player.hpp"
#include "Board.hpp"

class A
{
public:
  virtual void printMe() { printf("A"); };
};


class B : public A
{
public:
  void printMe() { printf("B"); };
};

int main(int argc, char * argv[])
{
  const int scale = 3;
  const Dimension2 real_screen_size = {801, 700};
  const Dimension2 scaled_screen_size = real_screen_size / scale;
  
  // set up game world
  Core core;
  Entity world("world", nullptr, nullptr, nullptr, nullptr);
  Board board("board");
  Player player("player");
  world.addChild(&board);
  world.addChild(&player);
  
  // initialize game world
  core.scale(scale);
  core.init(&world, "Q*bert", scaled_screen_size, {0x00, 0x00, 0x00, 0xFF});
  
  // game loop
  while (core.update());
  
  // destroy game world
  core.destroy();
  
  return 0;
}
