//
//  core.cpp
//  Game Engine
//

#include "core.hpp"
#include <stack>

// MARK: Helper functions

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

NotificationCenter & NotificationCenter::main()
{
  static NotificationCenter instance;
  return instance;
}

void NotificationCenter::notify(Event event, vector<GameObject*> * game_objects)
{
  for (auto f : _blocks[event]) f(event, game_objects);
}

size_t NotificationCenter::observe(Event event,
                                   function<void(Event,
                                                 vector<GameObject*>*)> block)
{
  auto size = _blocks[event].size();
  _blocks[event].push_back(block);
  return hash<string>{}(event.string_value() + to_string(size));
}

void NotificationCenter::unobserve(Event event, size_t id)
{
  auto blocks_for_event = _blocks[event];
  for (auto i = 0; i < blocks_for_event.size(); i++)
  {
    size_t h = hash<string>{}(event.string_value() + to_string(i));
    if (h == id)
    {
      _blocks[event].erase(blocks_for_event.begin()+i);
      return;
    }
  }
}


//
// MARK: - Core
//

//MARK: Member functions

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
  _reset = false;
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
  _initialized = false;
}

void Core::reset()
{
  _reset = true;
}

void Core::createTimer(double duration, function<void ()> block)
{
  _timers.push_back({elapsedTime() + duration, block});
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
    if (_reset)
    {
      root()->reset();
      _reset = false;
    }
    root()->update();
    
    // clear screen
    SDL_RenderPresent(renderer());
    SDL_RenderClear(renderer());
    
    // go through timers
    auto tmp = _timers;
    for (auto i = 0; i < tmp.size(); i++)
    {
      auto timer = tmp[i];
      const double current_time = elapsedTime();
      if (current_time >= timer.end_time)
      {
        timer.block();
        _timers.erase(_timers.begin()+i);
      }
    }
    
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

void Entity::update()
{
  _tmp_update(this, core(), 0);
  _tmp_update(this, core(), 1);
  _tmp_update(this, core(), 2);
  _tmp_update(this, core(), 3);
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
