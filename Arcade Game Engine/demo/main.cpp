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
  Core core(3);
  
  CoreOptions options {"Demo", 800, 700};
  core.changeBackgroundColor(0.2f, 0.2f, 0.2f);
  core.addControl("up",    SDLK_w);
  core.addControl("down",  SDLK_s);
  core.addControl("left",  SDLK_a);
  core.addControl("right", SDLK_d);
  
  // static cube
  Entity * staticCube = core.createEntity("staticCube");
  staticCube->translate({-0.5f, -5.0f, -25.0f});
//  staticCube->attachInputComponent(new CubeInputComponent);
  staticCube->attachColliderComponent(new SphereColliderComponent(0.49f));
  staticCube->attachRigidBodyComponent(new RigidBodyComponent);
  staticCube->attachGraphicsComponent(new CubeGraphicsComponent);
  
  // kinematic cube
  Entity * kinematicCube = core.createEntity("kinematicCube");
  kinematicCube->translate({-0.5f, 2.0f, -25.0f});
  kinematicCube->attachColliderComponent(new SphereColliderComponent(0.49f));
  kinematicCube->attachRigidBodyComponent(new CubeRigidBodyComponent);
  kinematicCube->attachGraphicsComponent(new CubeGraphicsComponent);
  
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
