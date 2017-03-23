//
//  types.cpp
//  Arcade Game Engine
//

#include "types.hpp"

Event::Event(string id) : id(id) {}

Event::Event(Event event, Parameter parameter)
  : id(event.id())
  , parameter(parameter)
{}

bool Event::operator==(Event event) const
{
  return id().compare(event.id()) == 0;
}

bool Event::operator<(Event event) const
{
  return id().compare(event.id()) < 0;
}
