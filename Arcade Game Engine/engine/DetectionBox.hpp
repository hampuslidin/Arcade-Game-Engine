//
//  DetectionBox.hpp
//  Arcade Game Engine
//

#pragma once

#include "core.hpp"

// Events


//
// MARK: - DetectionBoxPhysicsComponent
//

class DetectionBoxPhysicsComponent
  : public PhysicsComponent
{
public:
  DetectionBoxPhysicsComponent(Dimension2 dimension);
};


//
// MARK: - DetectionBox
//

class DetectionBox
  : public Entity
{
public:
  DetectionBox(string parent_id, int x, int y, int width, int height);
};
