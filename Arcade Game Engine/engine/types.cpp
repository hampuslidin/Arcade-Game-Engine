//
//  types.cpp
//  Arcade Game Engine
//

#include "types.hpp"

#include <cstring>

Event::Event(const char * id)
  : _id(id)
{}

Event::Event(const Event & event, const Parameter & parameter)
  : _id(event._id)
{
  this->parameter(parameter);
}

bool Event::operator==(const Event & event) const
{
  return strcmp(this->_id, event._id) == 0;
}

bool Event::operator<(const Event & event) const
{
  return strcmp(this->_id, event._id) < 0;
}
