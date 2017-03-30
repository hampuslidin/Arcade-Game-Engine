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
    float distance = 10.0f * core.deltaTime();
    float yaw      = -0.01f * core.mouseMovement().x;
    float pitch    = -0.01f * core.mouseMovement().y;
    
    if (yaw == 0 && pitch == 0)
    {
      printf("Zero rotation, %fs\n", core.elapsedTime());
    }
    
    entity()->rotate(yaw,   vec3(0.0f, 1.0f, 0.0f));
    entity()->rotate(pitch, entity()->localRight());
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
    vec3 worldRight(1.0f, 0.0f, 0.0f);
    vec3 worldUp(0.0f, 1.0f, 0.0f);
    float angle = 3.0f * core.deltaTime();
    
    if (core.checkKey("up"))    entity()->rotate(-angle, worldRight);
    if (core.checkKey("down"))  entity()->rotate(angle,  worldRight);
    if (core.checkKey("left"))  entity()->rotate(-angle, worldUp);
    if (core.checkKey("right")) entity()->rotate(angle,  worldUp);
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
  cube->translate(0.0f, 0.0f, -2.5f);
  cube->pInput(new CubeInputComponent);
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
