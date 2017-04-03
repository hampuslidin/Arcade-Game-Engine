//
//  main.cpp
//  Arcade Game Engine
//

#include "core.hpp"
#include <glm/gtx/transform.hpp>

const Event DidPressKey("DidPressKey");
const Event DidReleaseKey("DidReleaseKey");

class CameraInputComponent
  : public InputComponent
{
  
public:
  void update(const Core & core)
  {
    static float yaw   = 0.0f;
    static float pitch = 0.0f;
    yaw   -= 0.01f * core.mouseMovement().x;
    pitch -= 0.01f * core.mouseMovement().y;
    float distance = 10.0f * core.deltaTime();
    
//    entity()->setOrientation(pitch, yaw, 0.0f);

    // This code behaves weird, especially when moving the mouse really fast.
    entity()->rotate(-0.01f * core.mouseMovement().x, Core::WORLD_UP);
    entity()->rotate(-0.01f * core.mouseMovement().y, entity()->localRight());
    
    if (core.checkKey("up"))
      entity()->translate(distance, entity()->localForward());
    if (core.checkKey("down"))
      entity()->translate(distance, entity()->localBackward());
    if (core.checkKey("left"))
      entity()->translate(distance, entity()->localLeft());
    if (core.checkKey("right"))
      entity()->translate(distance, entity()->localRight());
  }
  
};

class CubeInputComponent
  : public InputComponent
{
  
public:
  void update(const Core & core)
  {
    const float angle = 3.0f * core.deltaTime();
    
    if (core.checkKey("up"))    entity()->rotate( angle, Core::WORLD_RIGHT);
    if (core.checkKey("down"))  entity()->rotate(-angle, Core::WORLD_RIGHT);
    if (core.checkKey("left"))  entity()->rotate(-angle, Core::WORLD_BACKWARD);
    if (core.checkKey("right")) entity()->rotate( angle, Core::WORLD_BACKWARD);
  }
  
};

class CubeRigidBodyComponent
  : public RigidBodyComponent
{

public:
  CubeRigidBodyComponent()
    : RigidBodyComponent()
  {
    setKinematic(true);
  }
  
};

class CubeGraphicsComponent
  : public GraphicsComponent
{
  
public:
  CubeGraphicsComponent()
    : GraphicsComponent()
  {
    vector<vec3> positions {
      {-0.5f, -0.5f, -0.5f},
      { 0.5f, -0.5f, -0.5f},
      {-0.5f,  0.5f, -0.5f},
      { 0.5f,  0.5f, -0.5f},
      {-0.5f, -0.5f,  0.5f},
      { 0.5f, -0.5f,  0.5f},
      {-0.5f,  0.5f,  0.5f},
      { 0.5f,  0.5f,  0.5f}
    };
    vector<vec3> colors {
      {0.0f, 0.0f, 0.0f},
      {1.0f, 0.0f, 0.0f},
      {0.0f, 1.0f, 0.0f},
      {1.0f, 1.0f, 0.0f},
      {0.0f, 0.0f, 1.0f},
      {1.0f, 0.0f, 1.0f},
      {0.0f, 1.0f, 1.0f},
      {1.0f, 1.0f, 1.0f}
    };
    vector<ivec3> indices {
      {0, 6, 2},
      {0, 4, 6},
      {1, 3, 7},
      {1, 7, 5},
      {2, 1, 0},
      {2, 3, 1},
      {3, 6, 7},
      {3, 2, 6},
      {4, 0, 1},
      {4, 1, 5},
      {5, 6, 4},
      {5, 7, 6}
    };
    
    attachMesh(positions, colors, indices);
    attachShader("shaders/simple.vert", "shaders/simple.frag");
  }
  
};

int main(int argc, char *  argv[])
{
  // core settings
  Core core(3);
  
  CoreOptions options {"Demo", 800, 700};
  core.changeBackgroundColor(0.2f, 0.2f, 0.2f);
  core.addControl("up",    SDLK_w);
  core.addControl("down",  SDLK_s);
  core.addControl("left",  SDLK_a);
  core.addControl("right", SDLK_d);
  
  // static cube
  Entity * staticCube = core.createEntity("staticCube");
  staticCube->translate(0.0f, 0.0f, -5.0f);
  staticCube->attachInputComponent(new CubeInputComponent);
  AABBColliderComponent * colliderAABB = new AABBColliderComponent;
  colliderAABB->resizeCollisionBox({-0.5f, -0.5f, -0.5f},
                                   { 0.5f,  0.5f,  0.5f});
  staticCube->attachColliderComponent(colliderAABB);
  staticCube->attachRigidBodyComponent(new RigidBodyComponent);
  staticCube->attachGraphicsComponent(new CubeGraphicsComponent);
  
  // kinematic cube
  Entity * kinematicCube = core.createEntity("kinematicCube");
  kinematicCube->translate(0.0f, 2.0f, -5.0f);
  colliderAABB = new AABBColliderComponent;
  colliderAABB->resizeCollisionBox({-0.5f, -0.5f, -0.5f},
                                   { 0.5f,  0.5f,  0.5f});
  kinematicCube->attachColliderComponent(colliderAABB);
  kinematicCube->attachRigidBodyComponent(new CubeRigidBodyComponent);
  kinematicCube->attachGraphicsComponent(new CubeGraphicsComponent);
  
  // camera
  core.camera()->attachInputComponent(new CameraInputComponent);
  
  if (core.init(options))
  {
    while (core.update());
    core.destroy();
  }
  return 0;
}
