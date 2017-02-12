//
//  core.cpp
//  Game Engine
//

#include "core.hpp"

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
void Notifier::addObserver(Observer * observer, Event event)
{
  addObserver(observer, (vector<Event>){event});
}

void Notifier::addObserver(Observer * observer, vector<Event> events)
{
  if (observer == nullptr) return;
  for (auto event : events) { observers()[event].push_back(observer); }
}

void Notifier::removeObserver(Observer * observer, Event event)
{
  if (event)
  {
    auto i = 0;
    auto observers_for_event = observers()[event];
    for (auto observer_for_event : observers_for_event)
    {
      if (observer == observer_for_event)
      {
        observers()[event].erase(observers_for_event.begin()+i);
        return;
      }
    }
  }
  else
  {
//    for (auto i = 0; i < observers().size(); i++)
//    {
//      if (observer == observers()[i])
//      {
//        observers().erase(observers().begin()+i);
//        return;
//      }
//    }
  }
}

void Notifier::notify(Entity & entity, Event event)
{
  for (auto observer : observers()[event]) observer->onNotify(entity, event);
}


/********************************
 * World
 ********************************/
World::World()
{
  scale(1);
}

bool World::init(const char * title,
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
  const int w_pos_x = dimensions.w < 0 ? SDL_WINDOWPOS_UNDEFINED : dimensions.w;
  const int w_pos_y = dimensions.h < 0 ? SDL_WINDOWPOS_UNDEFINED : dimensions.h;
  bounds({-1, -1, dimensions.w, dimensions.h});
  window(SDL_CreateWindow(title,
                          w_pos_x,
                          w_pos_y,
                          dimensions.w*scale(),
                          dimensions.h*scale(),
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
  _keys.up = _keys.down = _keys.left = _keys.right = _keys.fire = false;
  _prev_time = 0;
  _initialized = true;
  
  // initialize entities
  for (auto entity : entities())
  {
    entity->init(this);
  }
  
  return true;
}

void World::destroy()
{
  SDL_DestroyRenderer(renderer());
  SDL_DestroyWindow(window());
  SDL_Quit();
  _initialized = false;
}

void World::addEntity(Entity * entity)
{
  entities().push_back(entity);
  if (_initialized) entity->init(this);
}

void World::removeEntity(Entity * entity)
{
  for (auto i = 0; i < entities().size(); i++)
  {
    if (entity == entities()[i])
    {
      entities().erase(entities().begin()+i);
      return;
    }
  }
}

bool World::update()
{
  // record time
  float start_time = getElapsedTime();
  delta_time(start_time - _prev_time);
  _prev_time = start_time;
  
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
        case SDLK_SPACE:
          _keys.fire = true;
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
        case SDLK_SPACE:
          _keys.fire = false;
          break;
        case SDLK_ESCAPE:
        case SDLK_q:
          should_continue = false;
          break;
      }
    }
  }
  
  // update entities
  for (auto entity : entities()) (entity->update(*this));
  
  // clear screen
  SDL_RenderPresent(renderer());
  SDL_RenderClear(renderer());
  
  return should_continue;
}

bool World::resolveCollisions(Entity & collider, bool collision_response)
{
  bool has_collided = false;
  if (collider.physics())
  {
    Rectangle collider_cb = collider.physics()->collision_bounds();
    for (auto entity : entities())
    {
      if (entity == &collider) continue;
      if (entity->physics())
      {
        Rectangle entity_cb = entity->physics()->collision_bounds();
        Vector2 c_pos = collider.position();
        Vector2 e_pos = entity->position();
        float left_overlap  = c_pos.x + max_x(collider_cb) -
                              (e_pos.x + min_x(entity_cb));
        float up_overlap    = c_pos.y + max_y(collider_cb) -
                              (e_pos.y + min_y(entity_cb));
        float right_overlap = e_pos.x + max_x(entity_cb)   -
                              (c_pos.x + min_x(collider_cb));
        float down_overlap  = e_pos.y + max_y(entity_cb)   -
                              (c_pos.y + min_y(collider_cb));
        
        if (left_overlap > 0 && up_overlap > 0 && right_overlap > 0 && down_overlap > 0)
        {
          if (!collision_response) return true;
          has_collided = true;
          
          float * min = nullptr;
          for (auto f : {&left_overlap, &up_overlap, &right_overlap, &down_overlap})
          {
            if (min == 0 || *f < *min) min = f;
          }
          
          if (min == &left_overlap)
          {
            collider.moveBy(-left_overlap, 0);
            collider.changeHorizontalVelocityTo(0);
          }
          else if (min == &up_overlap)
          {
            collider.moveBy(0, -up_overlap);
            collider.changeVerticalVelocityTo(0);
          }
          else if (min == &right_overlap)
          {
            collider.moveBy(right_overlap, 0);
            collider.changeHorizontalVelocityTo(0);
          }
          else if (min == &down_overlap)
          {
            collider.moveBy(0, down_overlap);
            collider.changeVerticalVelocityTo(0);
          }
        }
      }
    }
  }
  return has_collided;
}

void World::getKeyStatus(World::KeyStatus & keys)
{
  keys.up    = this->_keys.up;
  keys.down  = this->_keys.down;
  keys.left  = this->_keys.left;
  keys.right = this->_keys.right;
  keys.fire  = this->_keys.fire;
}

float World::getElapsedTime()
{
  return SDL_GetTicks() / 1000.f;
}

/********************************
 * Entity
 ********************************/
Entity::Entity(InputComponent * input,
               PhysicsComponent * physics,
               GraphicsComponent * graphics)
{
  this->input(input);
  this->physics(physics);
  this->graphics(graphics);
}

Entity::~Entity()
{
  if (input())    delete input();
  if (physics())  delete physics();
  if (graphics()) delete graphics();
}

void Entity::init(World * owner)
{
  world(owner);
  if (input()) input()->init(this);
  if (physics()) physics()->init(this);
  if (graphics()) graphics()->init(this);
}

void Entity::update(World & world)
{
  if (input()) input()->update(world);
  if (physics()) physics()->update(world);
  if (graphics()) graphics()->update(world);
}

void Entity::moveTo(float x, float y)
{
  position().x = x;
  position().y = y;
}

void Entity::moveHorizontallyTo(float x)
{
  position().x = x;
}

void Entity::moveVerticallyTo(float y)
{
  position().y = y;
}

void Entity::moveBy(float dx, float dy)
{
  position().x += dx;
  position().y += dy;
}

void Entity::changeVelocityTo(float vx, float vy)
{
  velocity().x = vx;
  velocity().y = vy;
}

void Entity::changeHorizontalVelocityTo(float vx)
{
  velocity().x = vx;
}

void Entity::changeVerticalVelocityTo(float vy)
{
  velocity().y = vy;
}

void Entity::changeVelocityBy(float dvx, float dvy)
{
  velocity().x += dvx;
  velocity().y += dvy;
}


/********************************
 * Component
 ********************************/
void Component::init(Entity * owner)
{
  entity(owner);
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
                                    vector<const char *> files)
{
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
  if (sprites().size() > 0) current_sprite(sprites().at(0));
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
  bounds().dim.w = w;
  bounds().dim.h = h;
}

void GraphicsComponent::resizeBy(int dw, int dh)
{
  bounds().dim.w += dw;
  bounds().dim.h += dh;
}
