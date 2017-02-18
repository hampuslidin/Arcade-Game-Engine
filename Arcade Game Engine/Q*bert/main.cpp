//
//  main.cpp
//  Game Engine
//

#include "core.hpp"
#include "Level.hpp"
#include <stack>

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
  
  // print ids (debug)
  stack<Entity*> entity_stack;
  Entity * current_entity;
  entity_stack.push(&level);
  while (entity_stack.size() > 0)
  {
    current_entity = entity_stack.top();
    entity_stack.pop();
    
    printf("%s\n", current_entity->id().c_str());
    if (current_entity->input())
      printf("%s\n", current_entity->input()->id().c_str());
    if (current_entity->animation())
      printf("%s\n", current_entity->animation()->id().c_str());
    if (current_entity->physics())
      printf("%s\n", current_entity->physics()->id().c_str());
    if (current_entity->graphics())
      printf("%s\n", current_entity->graphics()->id().c_str());
    
    for (auto child : current_entity->children()) entity_stack.push(child);
  }
  
  // game loop
  while (core.update());
  
  // destroy game world
  core.destroy();
  
  return 0;
}
