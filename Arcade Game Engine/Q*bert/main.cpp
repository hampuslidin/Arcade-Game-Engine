//
//  main.cpp
//  Game Engine
//

#include "core.hpp"
#include "Level.hpp"

int main(int argc, char * argv[])
{
  const int scale = 3;
  const Dimension2 real_screen_size = {801, 700};
  const Dimension2 scaled_screen_size = real_screen_size / scale;
  
  // set up game world
  Core core;
  Level level("level");
  
  // initialize game world
  core.scale(scale);
  core.init(&level, "Q*bert", scaled_screen_size, {0x00, 0x00, 0x00, 0xFF});
  
  // game loop
  while (core.update());
  
  // destroy game world
  core.destroy();
  
  return 0;
}
