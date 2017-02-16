//
//  core.cpp
//  Game Engine
//

#include "core.hpp"
#include <fstream>
#include <stack>

/********************************
 * Sprite
 ********************************/
Sprite::Sprite(SDL_Renderer * renderer, SDL_Texture * texture)
  : _renderer(renderer)
  , _texture(texture)
{}

void Sprite::destroy()
{
  SDL_DestroyTexture(_texture);
}

void Sprite::draw(int x, int y, int w, int h, int scale)
{
  SDL_Rect rect {x*scale, y*scale, w*scale, h*scale};
  SDL_RenderCopy(_renderer, _texture, nullptr, &rect);
}

/********************************
 * Notifier
 ********************************/
void Notifier::addObserver(Observer * observer, const Event & event)
{
  observers()[event].push_back(observer);
}

void Notifier::removeObserver(Observer * observer, Event * event)
{
  if (event)
  {
    auto observers_for_event = observers()[*event];
    for (auto i = 0; i < observers_for_event.size(); i++)
    {
      if (observer == observers_for_event[i])
      {
        observers()[*event].erase(observers_for_event.begin()+i);
        return;
      }
    }
  }
  else
  {
    for (auto pair : observers())
    {
      for (auto i = 0; i < pair.second.size(); i++)
      {
        if (observer == pair.second[i])
        {
          observers()[pair.first].erase(pair.second.begin()+i);
          return;
        }
      }
    }
  }
}

void Notifier::notify(Entity & entity, Event event)
{
  for (auto observer : observers()[event]) observer->onNotify(entity, event);
}


/********************************
 * Core
 ********************************/
Core::Core()
{
  scale(1);
}

bool Core::init(Entity * root,
                const char * title,
                Dimension2 dimensions,
                RGBAColor background_color)
{
  // initialize SDL
  if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
  {
    SDL_Log("SDL_Init: %s\n", SDL_GetError());
    return false;
  }
  
  // initialize SDL_image
  if (IMG_Init(IMG_INIT_PNG) < 0)
  {
    SDL_Log("IMG_Init: %s\n", IMG_GetError());
    return false;
  }
  
  // create window
  const int w_pos_x = dimensions.x < 0 ? SDL_WINDOWPOS_UNDEFINED : dimensions.x;
  const int w_pos_y = dimensions.y < 0 ? SDL_WINDOWPOS_UNDEFINED : dimensions.y;
  view_dimensions({dimensions.x, dimensions.y});
  window(SDL_CreateWindow(title,
                          w_pos_x,
                          w_pos_y,
                          dimensions.x*scale(),
                          dimensions.y*scale(),
                          SDL_WINDOW_SHOWN));
  if (window() == nullptr)
  {
    SDL_Log("SDL_CreateWindow: %s\n", SDL_GetError());
    return false;
  }
  
  // create renderer for window
  renderer(SDL_CreateRenderer(window(), -1, SDL_RENDERER_ACCELERATED));
  if (renderer() == nullptr)
  {
    SDL_Log("SDL_CreateRenderer: %s\n", SDL_GetError());
    return false;
  }
  
  // clear screen
  SDL_SetRenderDrawColor(renderer(),
                         background_color.r,
                         background_color.g,
                         background_color.b,
                         background_color.a);
  SDL_RenderClear(renderer());
  
  // initialize member properties
  _keys.up = _keys.down = _keys.left = _keys.right = false;
  _prev_time = 0;
  
  // initialize entities
  this->root(root);
  root->init(this);
  stack<Entity*> entity_stack;
  entity_stack.push(root);
  Entity * current_entity;
  while (entity_stack.size())
  {
    current_entity = entity_stack.top();
    entity_stack.pop();
    _entities.push_back(current_entity);
    for (auto child : current_entity->children()) entity_stack.push(child);
  }
  
  return true;
}

void Core::destroy()
{
  root()->destroy();
    
  SDL_DestroyRenderer(renderer());
  SDL_DestroyWindow(window());
  SDL_Quit();
  _initialized = false;
}

bool Core::update()
{
  // record time
  double start_time = elapsedTime();
  delta_time(start_time - _prev_time);
  _prev_time = start_time;
  
  // check if initialized
  if (_initialized)
  {
    // check user input
    SDL_Event event;
    bool should_continue = true;
    while (SDL_PollEvent(&event))
    {
      if (event.type == SDL_QUIT)
      {
        should_continue = false;
        break;
      }
      if (event.type == SDL_KEYDOWN)
      {
        switch (event.key.keysym.sym)
        {
          case SDLK_UP:
            _keys.up = true;
            break;
          case SDLK_DOWN:
            _keys.down = true;
            break;
          case SDLK_LEFT:
            _keys.left = true;
            break;
          case SDLK_RIGHT:
            _keys.right = true;
            break;
        }
      }
      if (event.type == SDL_KEYUP)
      {
        switch (event.key.keysym.sym)
        {
          case SDLK_UP:
            _keys.up = false;
            break;
          case SDLK_DOWN:
            _keys.down = false;
            break;
          case SDLK_LEFT:
            _keys.left = false;
            break;
          case SDLK_RIGHT:
            _keys.right = false;
            break;
          case SDLK_ESCAPE:
          case SDLK_q:
            should_continue = false;
            break;
        }
      }
    }
  
    // update root
    root()->update();
    
    // clear screen
    SDL_RenderPresent(renderer());
    SDL_RenderClear(renderer());
    
    return should_continue;
  }
  _initialized = true;
  
  return true;
}

void Core::getKeyStatus(Core::KeyStatus & keys)
{
  keys.up    = this->_keys.up;
  keys.down  = this->_keys.down;
  keys.left  = this->_keys.left;
  keys.right = this->_keys.right;
}

double Core::elapsedTime()
{
  return SDL_GetTicks() / 1000.f;
}

/********************************
 * Entity
 ********************************/
Entity::Entity(string id,
               InputComponent * input,
               AnimationComponent * animation,
               PhysicsComponent * physics,
               GraphicsComponent * graphics)
{
  this->id(id);
  core(nullptr);
  parent(nullptr);
  
  this->input(input);
  this->animation(animation);
  this->physics(physics);
  this->graphics(graphics);
}

void Entity::init(Core * core)
{
  this->core(core);
  if (input()) input()->init(this);
  if (animation()) animation()->init(this);
  if (physics()) physics()->init(this);
  if (graphics()) graphics()->init(this);
  
  for (auto child : children())
  {
    child->init(core);
  }
}

void Entity::destroy()
{
  for (auto child : children())
  {
    child->destroy();
  }
  children().clear();
  
  if (input())     delete input();
  if (animation()) delete animation();
  if (physics())   delete physics();
  if (graphics())  delete graphics();
}

void Entity::addChild(Entity * child)
{
  children().push_back(child);
  child->parent(this);
}

Entity * Entity::findChild(string id)
{
  for (auto child : children())
  {
    if (child->id() == id)
    {
      return child;
    }
  }
  return nullptr;
}

void Entity::removeChild(string id)
{
  for (auto i = 0; i < children().size(); i++)
  {
    auto child = children()[i];
    if (child->id() == id)
    {
      child->parent(nullptr);
      children().erase(children().begin()+1);
    }
  }
}

void Entity::calculateWorldPosition(Vector2 & result)
{
  Vector2 world_position = local_position();
  Entity * current_entity = this;
  while ((current_entity = current_entity->parent()))
  {
    world_position += current_entity->local_position();
  }
  result.x = world_position.x;
  result.y = world_position.y;
}

void Entity::moveTo(double x, double y)
{
  local_position().x = x;
  local_position().y = y;
}

void Entity::moveHorizontallyTo(double x)
{
  local_position().x = x;
}

void Entity::moveVerticallyTo(double y)
{
  local_position().y = y;
}

void Entity::moveBy(double dx, double dy)
{
  local_position().x += dx;
  local_position().y += dy;
}

void Entity::changeVelocityTo(double vx, double vy)
{
  velocity().x = vx;
  velocity().y = vy;
}

void Entity::changeHorizontalVelocityTo(double vx)
{
  velocity().x = vx;
}

void Entity::changeVerticalVelocityTo(double vy)
{
  velocity().y = vy;
}

void Entity::changeVelocityBy(double dvx, double dvy)
{
  velocity().x += dvx;
  velocity().y += dvy;
}

void _tmp_update(Entity * root, Core * core, int c_type)
{
  Component * component = nullptr;
  
  stack<Entity*> entity_stack;
  entity_stack.push(root);
  Entity * current_entity;
  while (entity_stack.size())
  {
    current_entity = entity_stack.top();
    entity_stack.pop();
    
    switch (c_type) {
      case 0:
        component = current_entity->input();
        break;
      case 1:
        component = current_entity->animation();
        break;
      case 2:
        component = current_entity->physics();
        break;
      case 3:
        component = current_entity->graphics();
        break;
    }
    if (component) component->update(*core);
    
    if (current_entity->children().size() > 0)
    {
      for (long i = current_entity->children().size() - 1; i >= 0; i--)
      {
        entity_stack.push(current_entity->children()[i]);
      }
    }
    
  }
}

void Entity::update()
{
  _tmp_update(this, core(), 0);
  _tmp_update(this, core(), 1);
  _tmp_update(this, core(), 2);
  _tmp_update(this, core(), 3);
}


/********************************
 * Component
 ********************************/
void Component::init(Entity * entity)
{
  this->entity(entity);
}


/********************************
 * AnimationComponent
 ********************************/
AnimationComponent::AnimationComponent()
  : Component()
{
  animating(false);
  _calculate_velocity = false;
}

bool AnimationComponent::loadAnimationFromFile(string filename,
                                               string animation_id,
                                               double duration)
{
  vector<Vector2> movements;
  ifstream file(filename);
  if (file.is_open())
  {
    double x, y;
    while (file >> x >> y)
    {
      movements.push_back({x, y});
    }
    if (movements.size() > 0)
    {
      _animation_paths[animation_id] = {movements, duration};
      file.close();
      return true;
    }
  }
  file.close();
  return false;
}

bool AnimationComponent::removeAnimation(string id)
{
  return _animation_paths.erase(id) == 1;
}

bool AnimationComponent::performAnimation(string id, bool calculate_velocity)
{
  if (!animating() && _animation_paths.find(id) != _animation_paths.end())
  {
    animating(true);
    _animation = _animation_paths[id];
    _current_movement_index = 0;
    _animation_start_time = entity()->core()->elapsedTime();
    _calculate_velocity = calculate_velocity;
    notify(*entity(), DidStartAnimating);
    return true;
  }
  return false;
}

void AnimationComponent::update(Core & world)
{
  if (animating())
  {
    const double elapsed  = world.elapsedTime() - _animation_start_time;
    const double fraction = elapsed / _animation.duration;
    const long max = _animation.movements.size();
    const long bound = fraction < 1 ? floor(fraction * max) + 1 : max;
    
    Vector2 total_movement {0, 0};
    for (; _current_movement_index < bound; _current_movement_index++)
    {
      total_movement += _animation.movements[_current_movement_index];
    }
    
    entity()->moveBy(total_movement.x, total_movement.y);
    
    if (total_movement.x != 0 || total_movement.y != 0)
    {
      notify(*entity(), DidStartMovingInAnimation);
    }
    
    if (elapsed < _animation.duration) { return; }
    
    if (_calculate_velocity)
    {
      const Vector2 distance = _animation.movements.back();
      const double time = _animation.duration / _animation.movements.size();
      const Vector2 velocity = distance / time;
      entity()->changeVelocityTo(velocity.x, velocity.y);
    }
    animating(false);
    notify(*entity(), DidStopAnimating);
  }
}


/********************************
 * GraphicsComponent
 ********************************/
GraphicsComponent::~GraphicsComponent()
{
  for (auto sprite : sprites())
  {
    sprite->destroy();
  }
}

void GraphicsComponent::initSprites(SDL_Renderer & renderer,
                                    vector<const char *> files,
                                    int current_sprite_index)
{
  sprites().clear();
  for (auto file : files)
  {
    SDL_Surface * loaded_surface = IMG_Load(file);
    if (!loaded_surface)
    {
      SDL_Log("IMG_Load: %s\n", IMG_GetError());
    }
    else
    {
      SDL_Texture * player_texture =
      SDL_CreateTextureFromSurface(&renderer, loaded_surface);
      sprites().push_back(new Sprite(&renderer, player_texture));
      SDL_FreeSurface(loaded_surface);
    }
  }
  if (sprites().size() > 0) current_sprite(sprites()[current_sprite_index]);
}

void GraphicsComponent::offsetTo(int x, int y)
{
  bounds().pos.x = x;
  bounds().pos.y = y;
}

void GraphicsComponent::offsetBy(int dx, int dy)
{
  bounds().pos.x += dx;
  bounds().pos.y += dy;
}

void GraphicsComponent::resizeTo(int w, int h)
{
  bounds().dim.x = w;
  bounds().dim.y = h;
}

void GraphicsComponent::resizeBy(int dw, int dh)
{
  bounds().dim.x += dw;
  bounds().dim.y += dh;
}

void GraphicsComponent::update(Core & world)
{
  Vector2 entity_pos;
  entity()->calculateWorldPosition(entity_pos);
  current_sprite()->draw(entity_pos.x + bounds().pos.x,
                         entity_pos.y + bounds().pos.y,
                         bounds().dim.x,
                         bounds().dim.y,
                         world.scale());
}
