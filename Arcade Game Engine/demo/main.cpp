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
  void update(Core & core)
  {
    static float yaw   = 0.0f;
    static float pitch = 0.0f;
    yaw   -= 0.01f * core.mouseMovement().x;
    pitch -= 0.01f * core.mouseMovement().y;
    float distance = 10.0f * core.deltaTime();
    
    entity()->setOrientation(pitch, yaw, 0.0f);

    // This code behaves weird, especially when moving the mouse really fast.
//    entity()->rotate(-0.01f * core.mouseMovement().x, Core::WORLD_UP);
//    entity()->rotate(-0.01f * core.mouseMovement().y, entity()->localRight());
    
    if (core.checkKey("up"))    entity()->translate(0.0f,      0.0f, -distance);
    if (core.checkKey("down"))  entity()->translate(0.0f,      0.0f,  distance);
    if (core.checkKey("left"))  entity()->translate(-distance, 0.0f,  0.0f);
    if (core.checkKey("right")) entity()->translate( distance, 0.0f,  0.0f);
  }
  
};

class CubeInputComponent
  : public InputComponent
{
  
public:
  void update(Core & core)
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
    pIsKinematic(true);
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
    vector<int> indices {
      0, 6, 2,
      0, 4, 6,
      1, 3, 7,
      1, 7, 5,
      2, 1, 0,
      2, 3, 1,
      3, 6, 7,
      3, 2, 6,
      4, 0, 1,
      4, 1, 5,
      5, 6, 4,
      5, 7, 6
    };
    
    attachMesh(positions, colors, indices);
    attachShader("shaders/simple.vert", "shaders/simple.frag");
  }
  
};

int main(int argc, char *  argv[])
{
  Core core(2);
  CoreOptions options {"Demo", 800, 700};
  core.pBackgroundColor().r = 0.2f;
  core.pBackgroundColor().g = 0.2f;
  core.pBackgroundColor().b = 0.2f;
  core.addControl("up",    SDLK_w);
  core.addControl("down",  SDLK_s);
  core.addControl("left",  SDLK_a);
  core.addControl("right", SDLK_d);
  
  // cube
  Entity * cube = core.createEntity("cube");
  cube->translate(0.0f, 0.0f, -5.0f);
  cube->pInput(new CubeInputComponent);
//  cube->pRigidBody(new CubeRigidBodyComponent);
  cube->pGraphics(new CubeGraphicsComponent);
  
  // camera
  core.camera()->pInput(new CameraInputComponent);
  
  if (core.init(options))
  {
    while (core.update());
    core.destroy();
  }
  return 0;
}
