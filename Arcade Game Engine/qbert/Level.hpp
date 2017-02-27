//
//  Level.hpp
//  Arcade Game Engine
//

#pragma once

#include "core.hpp"

/**
 *  Defines a level.
 */
class Level : public Entity
{
public:
  prop_r<Level, bool> game_over;
  
  Level(string id);
  void reset();
};
