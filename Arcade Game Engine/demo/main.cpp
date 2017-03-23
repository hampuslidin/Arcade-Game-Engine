//
//  main.cpp
//  Arcade Game Engine
//

#include "core.hpp"
#include <glm/gtx/transform.hpp>

const Event DidPressKey("DidPressKey");
const Event DidReleaseKey("DidReleaseKey");

class CubeInputComponent
  : public InputComponent
{
  
public:
  void init(Entity * entity)
  {
    InputComponent::init(entity);
    
    _pressingUp    = false;
    _pressingDown  = false;
    _pressingLeft  = false;
    _pressingRight = false;
  }
  
  void update(Core & core)
  {
    Core::KeyStatus keys;
    core.keyStatus(keys);
    
    if (keys.up && !_pressingUp)
    {
      _pressingUp = true;
      NotificationCenter::notify(Event(DidPressKey, 0), *this);
    }
    else if (!keys.up && _pressingUp)
    {
      _pressingUp = false;
      NotificationCenter::notify(Event(DidReleaseKey, 0), *this);
    }
    
    if (keys.down && !_pressingDown)
    {
      _pressingDown = true;
      NotificationCenter::notify(Event(DidPressKey, 1), *this);
    }
    else if (!keys.down && _pressingDown)
    {
      _pressingDown = false;
      NotificationCenter::notify(Event(DidReleaseKey, 1), *this);
    }
    
    if (keys.left && !_pressingLeft)
    {
      _pressingLeft = true;
      NotificationCenter::notify(Event(DidPressKey, 2), *this);
    }
    else if (!keys.left && _pressingLeft)
    {
      _pressingLeft = false;
      NotificationCenter::notify(Event(DidReleaseKey, 2), *this);
    }
    
    if (keys.right && !_pressingRight)
    {
      _pressingRight = true;
      NotificationCenter::notify(Event(DidPressKey, 3), *this);
    }
    else if (!keys.right && _pressingRight)
    {
      _pressingRight = false;
      NotificationCenter::notify(Event(DidReleaseKey, 3), *this);
    }
  }
  
private:
  bool _pressingUp;
  bool _pressingDown;
  bool _pressingLeft;
  bool _pressingRight;
  
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
  
  void init(Entity * entity)
  {
    GraphicsComponent::init(entity);
    
    _pressingUp    = false;
    _pressingDown  = false;
    _pressingLeft  = false;
    _pressingRight = false;
    _rightAngle    = 0.0f;
    _upAngle       = 0.0f;
    
    auto eventHandler = [this](Event event)
    {
      switch (event.parameter())
      {
      case 0:
        _pressingUp = (event == DidPressKey);
        break;
      case 1:
        _pressingDown = (event == DidPressKey);
        break;
      case 2:
        _pressingRight = (event == DidPressKey);
        break;
      case 3:
        _pressingLeft = (event == DidPressKey);
        break;
      }
    };
    
    auto input = entity->input();
    NotificationCenter::observe(eventHandler, DidPressKey,   input);
    NotificationCenter::observe(eventHandler, DidReleaseKey, input);
  }
  
  mat4 modelMatrix()
  {
    mat4 matrix(1.0f);
    vec3 worldRight(1.0f, 0.0f, 0.0f);
    vec3 worldUp(0.0f, 1.0f, 0.0f);
    
    if      (_pressingUp  && !_pressingDown) _rightAngle -= 0.05f;
    else if (!_pressingUp &&  _pressingDown) _rightAngle += 0.05f;
    
    if      (_pressingLeft  && !_pressingRight) _upAngle -= 0.05f;
    else if (!_pressingLeft &&  _pressingRight) _upAngle += 0.05f;
    
    matrix = translate(matrix, vec3(0.0f, 0.0f, -2.5f));
    matrix = rotate(matrix, _rightAngle, worldRight);
    matrix = rotate(matrix, _upAngle, worldUp);
    
    return matrix;
  }
  
private:
  bool _pressingUp;
  bool _pressingDown;
  bool _pressingLeft;
  bool _pressingRight;
  
  float _rightAngle;
  float _upAngle;
  
};

int main(int argc, char *  argv[])
{
  Core core;
  CoreOptions options {"Demo", 800, 700};
  core.pBackgroundColor().r = 0.2f;
  core.pBackgroundColor().g = 0.2f;
  core.pBackgroundColor().b = 0.2f;
  
  Entity * cube = core.createEntity("cube");
  cube->addInput(new CubeInputComponent);
  cube->addGraphics(new CubeGraphicsComponent);
  
  if (core.init(options))
  {
    while (core.update());
    core.destroy();
  }
  return 0;
}
