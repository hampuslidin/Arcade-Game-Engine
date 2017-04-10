//
//  main.cpp
//  Arcade Game Engine
//

#include "core.hpp"
#include <glm/gtx/transform.hpp>


float r = 0.5f;

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
    vector<vec3> positions {
      {-r, -r, -r},
      { r, -r, -r},
      {-r,  r, -r},
      { r,  r, -r},
      {-r, -r,  r},
      { r, -r,  r},
      {-r,  r,  r},
      { r,  r,  r}
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
  int maxN = 5;
  int maxM = 5;
  Core core(maxN*maxM*2+1);
  
  CoreOptions options {"Demo", 800, 700};
  core.changeBackgroundColor(0.2f, 0.2f, 0.2f);
  core.addControl("up",    SDLK_w);
  core.addControl("down",  SDLK_s);
  core.addControl("left",  SDLK_a);
  core.addControl("right", SDLK_d);
  
  for (int n = 0; n < maxN; ++n)
  {
    for (int m = 0; m < maxM; ++m)
    {
      string indexString = to_string(n) + "_" + to_string(m);
      // static cube
      Entity * staticCube = core.createEntity("staticCube" + indexString);
      staticCube->translate
      ({
        (2*n-maxN)*r,
        (arc4random()/float(RAND_MAX)-10.0f)*r,
        (2*m-maxM)*r-25.0f
      });
      staticCube->attachInputComponent(new CubeInputComponent);
      staticCube->attachColliderComponent(new SphereColliderComponent(0.98*r));
      staticCube->attachRigidBodyComponent(new RigidBodyComponent);
      staticCube->attachGraphicsComponent(new CubeGraphicsComponent);
      
      // kinematic cube
      Entity * kinematicCube = core.createEntity("kinematicCube" + indexString);
      kinematicCube->translate
      ({
        (2*n-maxN)*r,
        (arc4random()/float(RAND_MAX)*4+4)*r,
        (2*m-maxM)*r-25.0f
      });
      kinematicCube->attachColliderComponent(new SphereColliderComponent(0.98*r));
      kinematicCube->attachRigidBodyComponent(new CubeRigidBodyComponent);
      kinematicCube->attachGraphicsComponent(new CubeGraphicsComponent);
    }
  }
  
  // camera
  core.camera()->attachInputComponent(new CameraInputComponent);
  core.camera()->attachColliderComponent(new SphereColliderComponent(15.0f));
  core.camera()->attachRigidBodyComponent(new CameraRigidBodyComponent);
  
  if (core.init(options))
  {
    while (core.update());
    core.destroy();
  }
  return 0;
}
