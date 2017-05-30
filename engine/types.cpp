//
//  types.cpp
//  Arcade Game Engine
//

#include "types.hpp"


// MARK: Properties
const string & Event::id() const { return _id; }
int Event::parameter() const     { return _parameter; }

// MARK: Member functions
Event::Event(string id) : _id(id) {}

Event::Event(Event event, int parameter)
  : _id(event._id)
  , _parameter(parameter)
{}

bool Event::operator==(Event event) const
{
  return id().compare(event.id()) == 0;
}

bool Event::operator<(Event event) const
{
  return id().compare(event.id()) < 0;
}
