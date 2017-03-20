//
//  main.cpp
//  Arcade Game Engine
//

#include "core.hpp"

int main(int argc, char *  argv[])
{
  Core core;
  Entity root("root", 0);
  int dimensions[] {800, 700};
  float background_color[] {0.2, 0.2, 0.8};
  if (core.init(&root, "Demo", dimensions, background_color))
  {
    while (core.update());
    core.destroy();
  }
  return 0;
}
