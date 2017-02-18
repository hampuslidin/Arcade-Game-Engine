//
//  types.cpp
//  Arcade Game Engine
//

#include "types.hpp"

#include <cstring>

string Event::string_value()
{
  return _id;
}

Event::Event(string id)
{
  _id = id;
}

Event::Event(Event event, Parameter parameter)
  : Event(event._id)
{
  this->parameter(parameter);
}

bool Event::operator==(Event event) const
{
  return _id.compare(event._id) == 0;
}

bool Event::operator<(Event event) const
{
  return _id.compare(event._id) < 0;
}
