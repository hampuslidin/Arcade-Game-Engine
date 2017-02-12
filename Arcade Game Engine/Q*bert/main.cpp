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
  
  auto upWallPhysics = new PhysicsComponent();
  upWallPhysics->collision_bounds({0, 0, 480, 480});
  upWallPhysics->dynamic(false);
  
  auto downWallPhysics = new PhysicsComponent();
  downWallPhysics->collision_bounds({0, 0, 480, 480});
  downWallPhysics->dynamic(false);
  
  auto leftWallPhysics = new PhysicsComponent();
  leftWallPhysics->collision_bounds({0, 0, 480, 480});
  leftWallPhysics->dynamic(false);
  
  auto rightWallPhysics = new PhysicsComponent();
  rightWallPhysics->collision_bounds({0, 0, 480, 480});
  rightWallPhysics->dynamic(false);
  
  Entity upWall(nullptr, upWallPhysics, nullptr);
  upWall.moveTo(0, -480);
  
  Entity downWall(nullptr, downWallPhysics, nullptr);
  downWall.moveTo(0, 480);
  
  Entity leftWall(nullptr, leftWallPhysics, nullptr);
  leftWall.moveTo(-480, 0);
  
  Entity rightWall(nullptr, rightWallPhysics, nullptr);
  rightWall.moveTo(480, 0);
  
  world.addEntity(&player);
  world.addEntity(&upWall);
  world.addEntity(&downWall);
  world.addEntity(&leftWall);
  world.addEntity(&rightWall);
  
  // initialize game world
  world.scale(1);
  world.init("Q*bert", {480, 480}, {0x00, 0x00, 0x00, 0xFF});
  
  // game loop
  while (world.update());
  
  // destroy game world
  world.destroy();
  
  return 0;
}
