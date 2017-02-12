//
//  main.cpp
//  Game Engine
//

#include "core.hpp"
#include "Player.hpp"

int main(int argc, char * argv[])
{
  // set up game world
  World world;
  Player player;
  world.addEntity(&player);
  
  // initialize game world
  world.init("Q*bert", {160, 160}, {0x00, 0x00, 0x00, 0xFF}, 4);
  
  // game loop
  while (world.update());
  
  // destroy game world
  world.destroy();
  
  return 0;
}
