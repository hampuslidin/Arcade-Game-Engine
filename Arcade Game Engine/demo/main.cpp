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
  void handleInput(const Core & core)
  {
    static float yaw   = 0.0f;
    static float pitch = 0.0f;
    yaw   -= 0.01f * core.mouseMovement().x;
    pitch -= 0.01f * core.mouseMovement().y;
    float distance = 2500.0f * core.deltaTime();
  
    const vec3 localUp       = entity()->localUp();
    const vec3 localRight    = entity()->localRight();
    const vec3 localBackward = entity()->localBackward();
    
    entity()->rotate(-0.01f * core.mouseMovement().x, localUp);
    entity()->rotate(-0.01f * core.mouseMovement().y, localRight);
    
    vec3 f(0.0f);
    if (core.checkKey("up"))    f -= distance*localBackward;
    if (core.checkKey("down"))  f += distance*localBackward;
    if (core.checkKey("left"))  f -= distance*localRight;
    if (core.checkKey("right")) f += distance*localRight;
    entity()->applyForce(f);
  }
  
};

class CameraRigidBodyComponent
  : public RigidBodyComponent
{
  
public:
  CameraRigidBodyComponent()
    : RigidBodyComponent()
  {
    setThermalVelocity(10.0f);
    setGravity(0.0f, 0.0f, 0.0f);
    setKinematic(true);
  }
};

class CubeInputComponent
  : public InputComponent
{
  
public:
  void handleInput(const Core & core)
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
    loadMeshFromObjFile("cube");
    loadShader("simple");
  }
  
};

int main(int argc, char *  argv[])
{
  // core settings
  Core core(19);
  
  CoreOptions options {"Demo", 800, 700};
  core.changeBackgroundColor(0.2f, 0.2f, 0.2f);
  core.addControl("up",    SDLK_w);
  core.addControl("down",  SDLK_s);
  core.addControl("left",  SDLK_a);
  core.addControl("right", SDLK_d);
  
  // high static cube
  Entity * highStaticCube = core.createEntity("highStaticCube");
  highStaticCube->translate({0.0f, -2.0f, -25.0f});
  highStaticCube->attachColliderComponent(new SphereColliderComponent(0.49f));
  highStaticCube->attachRigidBodyComponent(new RigidBodyComponent);
  highStaticCube->attachGraphicsComponent(new CubeGraphicsComponent);
  
  // middle static cubes
  int n = 0;
  for (int i = 0; i <= 1; ++i)
  {
    for (int j = -1; j <= 1; j += 2)
    {
      string id = "middleStaticCube" + to_string(n);
      Entity * middleStaticCube = core.createEntity(id);
      if (i == 0)
      {
        middleStaticCube->translate({j, -5.0f, -25.0f});
      } else
      {
        middleStaticCube->translate({0.0f, -5.0f, j-25.0f});
      }
      middleStaticCube->attachColliderComponent(new SphereColliderComponent(0.49f));
      middleStaticCube->attachRigidBodyComponent(new RigidBodyComponent);
      middleStaticCube->attachGraphicsComponent(new CubeGraphicsComponent);
      ++n;
    }
  }
  
  // low static cubes
  n = 0;
  for (int i = -1; i <= 1; i += 2)
  {
    for (int j = -1; j <= 1; j += 2)
    {
      string id = "lowStaticCube" + to_string(n);
      Entity * lowStaticCube = core.createEntity(id);
      lowStaticCube->translate({i, -8.0f, j-25.0f});
      lowStaticCube->attachColliderComponent(new SphereColliderComponent(0.49f));
      lowStaticCube->attachRigidBodyComponent(new RigidBodyComponent);
      lowStaticCube->attachGraphicsComponent(new CubeGraphicsComponent);
      ++n;
    }
  }
  
  // high bouncing cube
  Entity * highBouncingCube = core.createEntity("highBouncingCube");
  highBouncingCube->translate({0.0f, 8.0f, -25.0f});
  highBouncingCube->attachColliderComponent(new SphereColliderComponent(0.49f));
  highBouncingCube->attachRigidBodyComponent(new CubeRigidBodyComponent);
  highBouncingCube->attachGraphicsComponent(new CubeGraphicsComponent);
  
  // middle bouncing cubes
  n = 0;
  for (int i = 0; i <= 1; ++i)
  {
    for (int j = -1; j <= 1; j += 2)
    {
      string id = "middleBouncingCube" + to_string(n);
      Entity * middleBouncingCube = core.createEntity(id);
      if (i == 0)
      {
        middleBouncingCube->translate({j, 8.0f, -25.0f});
      } else
      {
        middleBouncingCube->translate({0.0f, 8.0f, j-25.0f});
      }
      middleBouncingCube->attachColliderComponent(new SphereColliderComponent(0.49f));
      middleBouncingCube->attachRigidBodyComponent(new CubeRigidBodyComponent);
      middleBouncingCube->attachGraphicsComponent(new CubeGraphicsComponent);
      ++n;
    }
  }
  
  // low bouncing cubes
  n = 0;
  for (int i = -1; i <= 1; i += 2)
  {
    for (int j = -1; j <= 1; j += 2)
    {
      string id = "lowBouncingCube" + to_string(n);
      Entity * lowBouncingCube = core.createEntity(id);
      lowBouncingCube->translate({i, 8.0f, j-25.0f});
      lowBouncingCube->attachColliderComponent(new SphereColliderComponent(0.49f));
      lowBouncingCube->attachRigidBodyComponent(new CubeRigidBodyComponent);
      lowBouncingCube->attachGraphicsComponent(new CubeGraphicsComponent);
      ++n;
    }
  }
  
  
  // camera
  core.camera()->attachInputComponent(new CameraInputComponent);
//  core.camera()->attachColliderComponent(new SphereColliderComponent(15.0f));
  core.camera()->attachRigidBodyComponent(new CameraRigidBodyComponent);
  
  if (core.init(options))
  {
    while (core.update());
    core.destroy();
  }
  return 0;
}
