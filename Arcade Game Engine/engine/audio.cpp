//
//  audio.cpp
//  Arcade Game Engine
//

#include <random>
#include <cmath>
#include "core.hpp"


//
// MARK: - AudioComponent
//

string AudioComponent::trait()
{
  return "audio";
}

AudioComponent::~AudioComponent()
{
  SDL_CloseAudio();
}

void AudioComponent::init(Entity * entity)
{
  Component::init(entity);
  
  _v = 0;
  frequency(440);
  
  auto fill_stream = [](void * userdata, uint8_t * stream, int length)
  {
    AudioComponent * audio = (AudioComponent*)userdata;
    for (int i = 0; i < length; i++)
    {
      stream[i] = 127*sin(audio->_v * 2 * M_PI / audio->_sample_rate);
      audio->_v += audio->frequency();
    }
  };
  
  SDL_AudioSpec desired_audio_spec, obtained_audio_spec;
  
  desired_audio_spec.freq     = _sample_rate;
  desired_audio_spec.format   = AUDIO_S8;
  desired_audio_spec.channels = 1;
  desired_audio_spec.samples  = 2048;
  desired_audio_spec.callback = fill_stream;
  desired_audio_spec.userdata = this;
  
  SDL_OpenAudio(&desired_audio_spec, &obtained_audio_spec);
  
  SDL_PauseAudio(0);
}

void AudioComponent::update(Core & core)
{
  random_device rd;
  mt19937 gen(rd());
  uniform_int_distribution<int> distribution(400, 700);
  frequency(distribution(gen));
}
