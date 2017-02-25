//
//  core.cpp
//  Game Engine
//

#include "core.hpp"
#include <stack>

// MARK: Helper functions

void _tmp_update(Entity * root, Core * core, int component_type)
{
  Component * component = nullptr;
  
  stack<Entity*> entity_stack;
  entity_stack.push(root);
  Entity * current_entity;
  while (entity_stack.size())
  {
    current_entity = entity_stack.top();
    entity_stack.pop();
    
    switch (component_type) {
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

//
// MARK: - Sprite
//

// MARK: Member functions

Sprite::Sprite(SDL_Renderer * renderer, SDL_Texture * texture)
  : _renderer(renderer)
  , _texture(texture)
{}

Sprite * Sprite::createSprite(SDL_Renderer * renderer, const char * filename)
{
  SDL_Surface * loaded_surface = IMG_Load(filename);
  if (!loaded_surface)
  {
    SDL_Log("IMG_Load: %s\n", IMG_GetError());
  }
  else
  {
    auto texture = SDL_CreateTextureFromSurface(renderer, loaded_surface);
    SDL_FreeSurface(loaded_surface);
    return new Sprite(renderer, texture);
  }
  return nullptr;
}

void Sprite::destroy()
{
  SDL_DestroyTexture(_texture);
}

void Sprite::draw(int x, int y, int w, int h, int scale)
{
  SDL_Rect rect {x*scale, y*scale, w*scale, h*scale};
  SDL_RenderCopy(_renderer, _texture, nullptr, &rect);
}

//
// MARK: - SpriteCollection
//

SpriteCollection & SpriteCollection::main()
{
  static SpriteCollection instance;
  return instance;
}

void SpriteCollection::init(SDL_Renderer * renderer)
{
  _renderer = renderer;
}

Sprite * SpriteCollection::create(string id, const char * filename)
{
  return _sprites[id] = Sprite::createSprite(_renderer, filename);
}

void SpriteCollection::destroy(string id)
{
  auto it = _sprites.find(id);
  if (it != _sprites.end())
  {
    _sprites.at(id)->destroy();
    _sprites.erase(it);
  }
}

void SpriteCollection::destroyAll()
{
  for (auto pair : _sprites)
  {
    pair.second->destroy();
  }
  _sprites.clear();
}

Sprite * SpriteCollection::retrieve(string id)
{
  if (_sprites.find(id) != _sprites.end())
  {
    return _sprites.at(id);
  }
  return nullptr;
}

void SpriteCollection::draw(string id, int x, int y, int w, int h, int scale)
{
  Sprite * sprite;
  if ((sprite = retrieve(id))) sprite->draw(x, y, w, h, scale);
}

//
// MARK: - NotificationCenter
//

void NotificationCenter::notify(Event event, GameObject & sender)
{
  for (auto pair : _instance()._blocks[event])
  {
    if (pair.second == nullptr || pair.second == &sender) pair.first(event);
  }
}

ObserverID NotificationCenter::observe(function<void(Event)> block,
                                       Event event,
                                       GameObject * sender)
{
  auto size = _instance()._blocks[event].size();
  _instance()._blocks[event].push_back({block, sender});
  return hash<string>{}(event.string_value() + to_string(size));
}

void NotificationCenter::unobserve(ObserverID id,
                                          Event event,
                                          GameObject * sender)
{
  auto blocks_for_event = _instance()._blocks[event];
  for (auto i = 0; i < blocks_for_event.size(); i++)
  {
    auto sender_for_block = blocks_for_event[i].second;
    if (sender_for_block == nullptr || sender_for_block == sender)
    {
      size_t h = hash<string>{}(event.string_value() + to_string(i));
      if (h == id)
      {
        _instance()._blocks[event].erase(blocks_for_event.begin()+i);
        return;
      }
    }
  }
}

// MARK: Private member functions
NotificationCenter & NotificationCenter::_instance()
{
  static NotificationCenter instance;
  return instance;
}

//
// MARK: - Core
//

// MARK: Member functions

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
  _key_status.up   = _key_status.down  = false;
  _key_status.left = _key_status.right = false;
  _reset = false;
  _pause = false;
  SpriteCollection::main().init(renderer());
  
  // initialize entities
  this->root(root);
  root->init(this);
  root->reset();
  
  return true;
}

void Core::destroy()
{
  SpriteCollection::main().destroyAll();
  root()->destroy();
    
  SDL_DestroyRenderer(renderer());
  SDL_DestroyWindow(window());
  SDL_Quit();
}

void Core::reset()
{
  _reset = true;
}

void Core::createTimer(double duration, function<void ()> block)
{
  _timers.push_back({effectiveElapsedTime() + duration, block});
}

bool Core::update()
{
  static double prev_time;
  
  // record time
  double start_time = elapsedTime();
  delta_time(start_time - prev_time);
  prev_time = start_time;
  
#ifdef GAME_ENGINE_DEBUG
  effectiveElapsedTime();
#endif
  
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
          _key_status.up = true;
          break;
        case SDLK_DOWN:
          _key_status.down = true;
          break;
        case SDLK_LEFT:
          _key_status.left = true;
          break;
        case SDLK_RIGHT:
          _key_status.right = true;
          break;
      }
    }
    if (event.type == SDL_KEYUP)
    {
      switch (event.key.keysym.sym)
      {
        case SDLK_UP:
          _key_status.up = false;
          break;
        case SDLK_DOWN:
          _key_status.down = false;
          break;
        case SDLK_LEFT:
          _key_status.left = false;
          break;
        case SDLK_RIGHT:
          _key_status.right = false;
          break;
        case SDLK_p:
          _pause = !_pause;
          effectiveElapsedTime();
          break;
        case SDLK_ESCAPE:
        case SDLK_q:
          should_continue = false;
          break;
      }
    }
  }
  
  // possibly do a reset
  if (!_pause && _reset)
  {
    _timers.clear();
    root()->reset();
    _reset = false;
  }
  
  // update root
  if (!_pause) root()->update();
  else         root()->update(0b0001);
  
#ifdef GAME_ENGINE_DEBUG
  // draw bounding boxes
  RGBAColor prev_color;
  SDL_GetRenderDrawColor(renderer(),
                         &prev_color.r,
                         &prev_color.g,
                         &prev_color.b,
                         &prev_color.a);
  SDL_SetRenderDrawColor(renderer(), 0xFF, 0xFF, 0xFF, 0xFF);
  stack<Entity*> entity_stack;
  entity_stack.push(root());
  while (entity_stack.size() > 0)
  {
    Entity * current_entity = entity_stack.top();
    entity_stack.pop();
    
    PhysicsComponent * current_physics_component;
    if ((current_physics_component = current_entity->physics()))
    {
      SDL_Rect rect;
      Rectangle bounds = current_physics_component->collision_bounds();
      Vector2 world_position;
      current_entity->calculateWorldPosition(world_position);
      rect.x = (world_position.x + bounds.pos.x) * scale();
      rect.y = (world_position.y + bounds.pos.y) * scale();
      rect.w = bounds.dim.x * scale();
      rect.h = bounds.dim.y * scale();
      SDL_RenderDrawRect(renderer(), &rect);
    }
    
    for (auto child : current_entity->children())
    {
      entity_stack.push(child);
    }
  }
  SDL_SetRenderDrawColor(renderer(),
                         prev_color.r,
                         prev_color.g,
                         prev_color.b,
                         prev_color.a);
#endif
  
  // clear screen
  SDL_RenderPresent(renderer());
  SDL_RenderClear(renderer());
  
  // go through timers
  if (!_pause)
  {
    int i = 0;
    while (i < _timers.size())
    {
      auto timer = _timers[i];
      const double current_time = effectiveElapsedTime();
      if (current_time >= timer.end_time)
      {
        timer.block();
        _timers.erase(_timers.begin() + i);
      }
      else i++;
    }
  }

  return should_continue;

  return true;
}

void Core::keyStatus(Core::KeyStatus & key_status)
{
  key_status.up    = _key_status.up;
  key_status.down  = _key_status.down;
  key_status.left  = _key_status.left;
  key_status.right = _key_status.right;
}

double Core::elapsedTime()
{
  return SDL_GetTicks() / 1000.f;
}

double Core::effectiveElapsedTime()
{
  static double last_pause_time;
  static double total_pause_duration;
  static bool pause_toggle;
  
  const double elapsed = elapsedTime();
  
  if (_pause && !pause_toggle)
  {
    pause_toggle = true;
    last_pause_time = elapsed;
    
#ifdef GAME_ENGINE_DEBUG
    printf("/**************** PAUSED ****************/\n");
#endif
    
  }
  else if (!_pause && pause_toggle)
  {
    pause_toggle = false;
    total_pause_duration += elapsed - last_pause_time;
    
#ifdef GAME_ENGINE_DEBUG
    printf("/**************** RESUMED ***************/\n");
#endif
    
  }
  
#ifdef GAME_ENGINE_DEBUG
  static double last_print_time;
  
  if (elapsed - last_print_time >= 0.1)
  {
    printf("Elapsed: %f\t\t", elapsed);
    printf("Effective elapsed: %f\t\t",
           !_pause
             ? elapsed - total_pause_duration
             : last_pause_time - total_pause_duration);
    printf("Last pause time: %f\t\t", last_pause_time);
    printf("Total pause duration: %f\n",
           !_pause
             ? total_pause_duration
             : total_pause_duration + elapsed - last_pause_time);
    last_print_time = elapsed;
  }
#endif

  return (!_pause ? elapsed : last_pause_time) - total_pause_duration;
}


//
// MARK: - Entity
//

// MARK: Property functions

string Entity::id()
{
  return _id;
}

void Entity::order(int order)
{
  if (parent())
  {
    auto tmp = parent();
    tmp->removeChild(id());
    tmp->addChild(this, order);
  }
}

int Entity::order()
{
  if (parent())
  {
    for (int i = 0; i < parent()->children().size(); i++)
    {
      auto child = parent()->children()[i];
      if (child->id() == id())
      {
        return i;
      }
    }
  }
  return 0;
}


// MARK: Member functions

Entity::Entity(string id,
               InputComponent * input,
               AnimationComponent * animation,
               PhysicsComponent * physics,
               GraphicsComponent * graphics)
{
  _id = id;
  core(nullptr);
  parent(nullptr);
  local_position({0, 0});
  
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
  
  for (auto child : children()) child->init(core);
}

void Entity::reset()
{
  velocity({0, 0});
  
  if (input()) input()->reset();
  if (animation()) animation()->reset();
  if (physics()) physics()->reset();
  if (graphics()) graphics()->reset();
  
  for (auto child : children()) child->reset();
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

void Entity::addChild(Entity * child, int order)
{
  if (order >= 0)
  {
    if (order > children().size()) order = (int)children().size();
    children().insert(children().begin()+order, child);
  }
  else
  {
    children().push_back(child);
  }
  child->parent(this);
}

Entity * Entity::findChild(string id)
{
  for (auto child : children())
  {
    if (child->id().compare(id) == 0) return child;
    auto possible_find = child->findChild(id);
    if (possible_find) return possible_find;
  }
  return nullptr;
}

void Entity::removeChild(string id)
{
  for (int i = 0; i < children().size(); i++)
  {
    auto child = children()[i];
    if (child->id() == id)
    {
      child->parent(nullptr);
      children().erase(children().begin()+i);
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

void Entity::update(uint8_t mask)
{
  uint8_t use_component = mask;
  int component_type = 3;
  while (use_component > 0) {
    if (use_component & 0b0001) _tmp_update(this, core(), component_type--);
    use_component >>= 1;
  }
}


//
// MARK: - Component
//

// MARK: Property functions

string Component::id()
{
  string id = trait() + "_component";
  if (entity()) id = entity()->id() + "_" + id;
  return id;
}

// MARK: Member functions
void Component::init(Entity * entity)
{
  this->entity(entity);
}


//
// MARK: - InputComponent
//

// MARK: Property functions

string InputComponent::trait() { return "input"; }


//
// MARK: - GraphicsComponent
//

// MARK: Property functions

string GraphicsComponent::trait() { return "graphics"; }

// MARK: Member functions

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
  if (current_sprite())
  {
    Vector2 entity_pos;
    entity()->calculateWorldPosition(entity_pos);
    current_sprite()->draw(entity_pos.x + bounds().pos.x,
                           entity_pos.y + bounds().pos.y,
                           bounds().dim.x,
                           bounds().dim.y,
                           world.scale());
  }
}
