//
//  Wrongway.cpp
//  Game Engine
//

#include "Wrongway.hpp"


//
// MARK: - WrongwayInputComponent
//

// MARK: Member functions

ControllerDirection WrongwayInputComponent::update_direction(Core & core)
{
  return arc4random_uniform(2)*3;
}

double WrongwayInputComponent::animation_ending_delay()
{
  return 0.2;
};

vector<pair<int, int>> WrongwayInputComponent::board_position_changes()
{
  return {
    {-1, 0},
    {     },
    {     },
    { 0, 1}
  };
}

//
// MARK: - WrongwayAnimationComponent
//

// MARK: Member functions

vector<Vector2> WrongwayAnimationComponent::end_points()
{
  return {
    {16, -24},
    {       },
    {       },
    {32,   0}
  };
}

double WrongwayAnimationComponent::animation_speed()
{
  return 0.7;
};


//
// MARK: - WrongwayPhysicsComponent
//

// MARK: Member functions

WrongwayPhysicsComponent::WrongwayPhysicsComponent()
  : ControllerPhysicsComponent()
{
  this->gravity({1.417, -0.818});
}

void WrongwayPhysicsComponent::init(Entity * entity)
{
  ControllerPhysicsComponent::init(entity);
  
  auto did_move_out_of_view = [entity](Event)
  {
    entity->enabled(false);
    entity->reset();
  };
  
  NotificationCenter::observe(did_move_out_of_view, DidMoveOutOfView, this);
}


//
// MARK: - Wrongway
//

Wrongway::Wrongway()
  : Controller("enemy_wrongway", default_order())
{
  addInput(new WrongwayInputComponent());
  addAnimation(new WrongwayAnimationComponent());
  addPhysics(new WrongwayPhysicsComponent());
  addGraphics(new WrongwayGraphicsComponent());
}

void Wrongway::reset()
{
  Controller::reset();
  
  enabled(false);
  board_position(default_board_position());
  order(default_order());
  direction(default_direction());
  
  core()->createEffectiveTimer(arc4random_uniform(4)+1, [this]
  {
   enabled(true);
  });
  
  const Dimension2 view_dimensions = core()->view_dimensions();
  moveTo(view_dimensions.x/2 - 118, view_dimensions.y-30);
}

string Wrongway::prefix_standing()
{
  return "enemy_wrongway_standing";
}

string Wrongway::prefix_jumping()
{
  return "enemy_wrongway_jumping";
}

int Wrongway::direction_mask()
{
  return 0b1001;
}

pair<int, int> Wrongway::default_board_position()
{
  return {6, 0};
}

int Wrongway::default_order()
{
  return 91;
}

ControllerDirection Wrongway::default_direction()
{
  return UP;
}
