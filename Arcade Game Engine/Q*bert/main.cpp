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
  
  const int screen_size = 200;
  const int size = screen_size;
  
  auto upWallPhysics = new PhysicsComponent();
  upWallPhysics->collision_bounds({0, 0, size, size});
  upWallPhysics->dynamic(false);
  
  auto downWallPhysics = new PhysicsComponent();
  downWallPhysics->collision_bounds({0, 0, size, size});
  downWallPhysics->dynamic(false);
  
  auto leftWallPhysics = new PhysicsComponent();
  leftWallPhysics->collision_bounds({0, 0, size, size});
  leftWallPhysics->dynamic(false);
  
  auto rightWallPhysics = new PhysicsComponent();
  rightWallPhysics->collision_bounds({0, 0, size, size});
  rightWallPhysics->dynamic(false);
  
  Entity upWall(nullptr, upWallPhysics, nullptr);
  upWall.moveTo((screen_size-size)/2, -size);
  
  Entity downWall(nullptr, downWallPhysics, nullptr);
  downWall.moveTo((screen_size-size)/2, screen_size);
  
  Entity leftWall(nullptr, leftWallPhysics, nullptr);
  leftWall.moveTo(-size, (screen_size-size)/2);
  
  Entity rightWall(nullptr, rightWallPhysics, nullptr);
  rightWall.moveTo(screen_size, (screen_size-size)/2);
  
  world.addEntity(&player);
  world.addEntity(&upWall);
  world.addEntity(&downWall);
  world.addEntity(&leftWall);
  world.addEntity(&rightWall);
  
  // initialize game world
  world.scale(3);
  world.init("Q*bert", {screen_size, screen_size}, {0x00, 0x00, 0x00, 0xFF});
  
  // game loop
  while (world.update());
  
  // destroy game world
  world.destroy();
  
  return 0;
}
