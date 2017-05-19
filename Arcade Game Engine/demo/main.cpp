//
//  main.cpp
//  Arcade Game Engine
//

#include "core.hpp"
#include <glm/gtx/transform.hpp>
#include <random>


////////////////////////////////////////
// INPUT COMPONENTS
////////////////////////////////////////

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

class SpinInputComponent
  : public InputComponent
{
  
public:
  void handleInput(const Core & core)
  {
    const float angle = 0.5f * core.deltaTime();
    
    if (core.checkKey("up"))    entity()->rotate( angle, Core::WORLD_RIGHT);
    if (core.checkKey("down"))  entity()->rotate(-angle, Core::WORLD_RIGHT);
    if (core.checkKey("left"))  entity()->rotate(-angle, Core::WORLD_BACKWARD);
    if (core.checkKey("right")) entity()->rotate( angle, Core::WORLD_BACKWARD);
  }
  
};

class DesertLightInputComponent
  : public InputComponent
{
  
public:
  DesertLightInputComponent(default_random_engine * generator)
    : InputComponent()
    , _generator(generator)
    , _initialized(false)
  {}
  
  void handleInput(const Core & core)
  {
    if (!_initialized)
    {
      const Entity * desert = core.findEntity("desert");
      const vector<float> & desertVertices = desert->graphics()->vertices();
      int numberOfDesertVertices = (int)desert->graphics()->numberOfVertices();
      uniform_int_distribution<int> distribution(0, numberOfDesertVertices-1);
      int r = distribution(*_generator);
      vec3 v = vec3(desert->worldTransform() * vec4(desertVertices[3*r],
                                                    desertVertices[3*r+1],
                                                    desertVertices[3*r+2],
                                                    1.0f));
      v.y += 1.0f;
      entity()->reposition(v);
      _initialized = true;
    }
  }
  
private:
  default_random_engine * _generator;
  vec3 _v;
  bool _initialized;
};

class OrbitInputController
  : public InputComponent
{
  
public:
  OrbitInputController(float radius)
    : InputComponent()
    , _radius(radius)
  {}
  
  void handleInput(const Core & core)
  {
    const double t = core.effectiveElapsedTime();
    const double T = 5.0;
    const float p = 2*M_PI*t/T;
    vec3 v(_radius*cos(p), 0.0f, _radius*sin(p));
    quat q(1.0f, 0.0f, 0.0f, 0.0f);
    q = glm::rotate(q, p/0.2718f, Core::WORLD_RIGHT);
    q = glm::rotate(q, p/0.3141f, Core::WORLD_BACKWARD);
    v = glm::rotate(q, v);
    entity()->reposition(v);
  }
  
private:
  float _radius;
  
};

////////////////////////////////////////
// GRAPHICS COMPONENTS
////////////////////////////////////////

class LightGraphicsComponent
  : public GraphicsComponent
{
  
public:
  void init(Entity * entity)
  {
    GraphicsComponent::init(entity);
    
    auto interpolate = [](float a, float b, float p) { return (1-p)*a+p*b; };
    
    float r = 0.0f, g = 0.0f, b = 0.0f;
    float h = (float)arc4random()/RAND_MAX*360.0f, s = 0.75f, l = 0.75f;
    
    // hue
    float x, y;
    const float p = h/60.0f;
    const int   i = p;
    if (i % 2 == 0)
    {
      x = 1.0f;
      y = p-(i/2)*2.0f;
    }
    else
    {
      x = (i/2)*2.0f-p;
      y = 1.0f;
    }
    if (h < 120.0f)
    {
      r = x;
      g = y;
    }
    else if (h < 240.0f)
    {
      g = x;
      b = y;
    }
    else
    {
      b = x;
      r = y;
    }
    
    // saturation
    r = interpolate(l, r, s);
    g = interpolate(l, g, s);
    b = interpolate(l, b, s);
    
    // lightness
    if (l < 0.5f)
    {
      r = interpolate(0.0f, r, 2*l);
      g = interpolate(0.0f, g, 2*l);
      b = interpolate(0.0f, b, 2*l);
    }
    else
    {
      r = interpolate(r, 1.0f, 2*l-1.0f);
      g = interpolate(g, 1.0f, 2*l-1.0f);
      b = interpolate(b, 1.0f, 2*l-1.0f);
    }

    diffuseColor({r, g, b});
    loadShader("shaders/default.vert", "shaders/default.frag");
    loadObject("objects/sphere.obj");
  }
  
};

class DeferredGraphicsComponent
  : public GraphicsComponent
{
  
public:
  DeferredGraphicsComponent(const string & objectFileName,
                            const string & diffuseMapFileName)
    : GraphicsComponent()
    , _objectFileName(objectFileName)
    , _diffuseMapFileName(diffuseMapFileName)
  {}
  
  void init(Entity * entity)
  {
    GraphicsComponent::init(entity);
    
    loadShader("shaders/deferred_geometry.vert",
               "shaders/deferred_geometry.frag");
    loadObject(_objectFileName.c_str());
    loadTexture(_diffuseMapFileName.c_str(), Diffuse);
  }
  
private:
  string _objectFileName;
  string _diffuseMapFileName;
  
};

////////////////////////////////////////
// RIGID BODY COMPONENTS
////////////////////////////////////////

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

class KinematicRigidBodyComponent
  : public RigidBodyComponent
{

public:
  KinematicRigidBodyComponent()
    : RigidBodyComponent()
  {
    setKinematic(true);
  }
  
};

////////////////////////////////////////
// MAIN PROGRAM
////////////////////////////////////////

int main(int argc, char *  argv[])
{
  // core settings
  Core core(42);
  CoreOptions options {"Demo", 1280, 756};
  core.changeBackgroundColor(0.2f, 0.2f, 0.2f);
  core.addControl("up",    SDLK_w);
  core.addControl("down",  SDLK_s);
  core.addControl("left",  SDLK_a);
  core.addControl("right", SDLK_d);
  
  // desert
  Entity * desert = core.createEntity("desert");
  desert->attachGraphicsComponent(new DeferredGraphicsComponent("objects/desert.obj", "textures/desert_diffuse.png"));
  desert->translate({-50.0f, -25.0f, 10.0f});
  desert->rotate(M_PI/2, Core::WORLD_LEFT);
  desert->scale(100.0f);
  
  // high static cube
  Entity * highStaticCube = core.createEntity("highStaticCube");
  highStaticCube->attachColliderComponent(new SphereColliderComponent(0.99f));
  highStaticCube->attachRigidBodyComponent(new RigidBodyComponent);
  highStaticCube->attachGraphicsComponent(new DeferredGraphicsComponent("objects/cube.obj", "textures/cube_diffuse.png"));
  highStaticCube->translate({0.0f, 0.0f, -15.0f});
  highStaticCube->scale(2.0f);
  
  // middle static cubes
  for (int i = 0; i < 2; ++i)
  {
    Entity * middleStaticCube = core.createEntity("middleStaticCube" + to_string(i));
    middleStaticCube->attachColliderComponent(new SphereColliderComponent(0.99f));
    middleStaticCube->attachRigidBodyComponent(new RigidBodyComponent);
    middleStaticCube->attachGraphicsComponent(new DeferredGraphicsComponent("objects/cube.obj", "textures/cube_diffuse.png"));
    middleStaticCube->translate({2.0f*(2*i-1), -2.0f, -15.0f});
    middleStaticCube->scale(2.0f);
  }
  
  // low static cube
  Entity * lowStaticCube = core.createEntity("lowStaticCube");
  lowStaticCube->attachColliderComponent(new SphereColliderComponent(0.99f));
  lowStaticCube->attachRigidBodyComponent(new RigidBodyComponent);
  lowStaticCube->attachGraphicsComponent(new DeferredGraphicsComponent("objects/cube.obj", "textures/cube_diffuse.png"));
  lowStaticCube->translate({0.0f, -4.0f, -13.0f});
  lowStaticCube->scale(2.0f);
  
  // high bouncing light
  Entity * highBouncingLight = core.createEntity("highBouncingLight", "root", Light);
  highBouncingLight->attachColliderComponent(new SphereColliderComponent(0.99f));
  highBouncingLight->attachRigidBodyComponent(new KinematicRigidBodyComponent);
  highBouncingLight->attachGraphicsComponent(new LightGraphicsComponent);
  highBouncingLight->translate({0.0f, 5.0f, -15.0f});
  
  // middle bouncing lights
  for (int i = 0; i < 2; ++i)
  {
    Entity * middleBouncingLight = core.createEntity("middleBouncingLight" + to_string(i), "root", Light);
    middleBouncingLight->attachColliderComponent(new SphereColliderComponent(0.99f));
    middleBouncingLight->attachRigidBodyComponent(new KinematicRigidBodyComponent);
    middleBouncingLight->attachGraphicsComponent(new LightGraphicsComponent);
    middleBouncingLight->translate({2.0f*(2*i-1), 5.0f, -15.0f});
  }
  
  // low bouncing light
  Entity * lowBouncingLight = core.createEntity("lowBouncingLight", "root", Light);
  lowBouncingLight->attachColliderComponent(new SphereColliderComponent(0.99f));
  lowBouncingLight->attachRigidBodyComponent(new KinematicRigidBodyComponent);
  lowBouncingLight->attachGraphicsComponent(new LightGraphicsComponent);
  lowBouncingLight->translate({0.0f, 5.0f, -13.0f});
  
  // earth
  Entity * earth = core.createEntity("earth");
  earth->attachInputComponent(new SpinInputComponent);
  earth->attachGraphicsComponent(new DeferredGraphicsComponent("objects/sphere.obj", "textures/earth_diffuse.png"));
  earth->translate({0.0f, 0.0f, -50.0f});
  earth->rotate(M_PI/2, Core::WORLD_UP);
  earth->scale(5.0f);
  
  // orbiting light
  Entity * orbitingLight = core.createEntity("orbitingLight", "earth", Light);
  orbitingLight->scale(0.1f);
  orbitingLight->attachInputComponent(new OrbitInputController(5.5f));
  orbitingLight->attachGraphicsComponent(new LightGraphicsComponent);
  
  // desert lights
  default_random_engine generator;
  for (int i = 0; i < 30; ++i)
  {
    Entity * desertLight = core.createEntity("desertLight" + to_string(i), "root", Light);
    desertLight->scale(0.5f);
    desertLight->attachInputComponent(new DesertLightInputComponent(&generator));
    desertLight->attachGraphicsComponent(new LightGraphicsComponent);
  }
  
  // camera
  core.camera()->attachInputComponent(new CameraInputComponent);
  core.camera()->attachRigidBodyComponent(new CameraRigidBodyComponent);
  
  if (core.init(options))
  {
    while (core.update());
    core.destroy();
  }
  return 0;
}
