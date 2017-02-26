//
//  DetectionBox.cpp
//  Arcade Game Engine
//

#include "DetectionBox.hpp"

//
// MARK: - DetectionBoxPhysicsComponent
//

DetectionBoxPhysicsComponent::DetectionBoxPhysicsComponent(Dimension2 dimension)
{
  collision_bounds({0, 0, dimension});
  collision_detection(true);
}


//
// MARK: - DetectionBox
//

DetectionBox::DetectionBox(string parent_id,
                           int x,
                           int y,
                           int width,
                           int height)
  : Entity(parent_id + "_detection_box",
           nullptr,
           nullptr,
           new DetectionBoxPhysicsComponent({(double)width, (double)height}),
           nullptr)
{
  moveTo(x, y);
}
