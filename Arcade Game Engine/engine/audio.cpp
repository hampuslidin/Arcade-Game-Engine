//
//  audio.cpp
//  Arcade Game Engine
//

#include <algorithm>
#include "tinyxml2.h"
#include "core.hpp"
#include "Player.hpp"

using namespace tinyxml2;

//
// MARK: - Synthesizer
//

Synthesizer::_Operator::_Operator(double frequency,
                                  double modulation_index,
                                  WaveType wave_type,
                                  double threshold_low,
                                  double threshold_high,
                                  maybe<double> pitch_glide)
  : frequency(frequency)
  , modulation_index(modulation_index)
  , wave_type(wave_type)
  , threshold_low(threshold_low)
  , threshold_high(threshold_high)
  , modulators()
  , pitch_glide(pitch_glide)
  , _prev_freq(frequency)
  , _prev_gliss_freq(pitch_glide() ? *pitch_glide() : 0)
  , _log_freq(log(frequency))
  , _log_gliss_freq(pitch_glide() ? log(*pitch_glide()) : 0)
{}

void Synthesizer::_Operator::addModulator(_Operator * modulator)
{
  modulators.push_back(modulator);
}

double Synthesizer::_Operator::calculateSample(double time, double duration)
{
  double phase = _calculatePhase(time, duration);
  double sample;
  
  double glide_freq = frequency;
  if (pitch_glide())
  {
    const double p = time / duration;
    const auto interval = _toneInterval();
    const double tone = interval.first*(1-p) + interval.second*p;
    
    // this adjustment factor, which was found by trial and error, allows for 3
    // octaves of pitch bending without the signal going bananas
    const double k = 0.6025 + 0.0845*((interval.first-interval.second)/12-1);
    
    glide_freq = (1-k) * frequency + k * 440 * pow(2, tone/12);
    // TODO: adjust for time
  }
  
  switch (wave_type)
  {
    case SMOOTH:
      sample = sin(2* M_PI * glide_freq * time + phase);
      break;
    case SQUARE:
      sample = 2*((int)(glide_freq*2*time + phase/M_PI) % 2)-1;
      break;
    case TRIANGLE:
      sample = 2*fabs(1-2*fmod(glide_freq*time + phase/M_PI, 1))-1;
      break;
    case SAWTOOTH:
      sample = 1-2*fmod(glide_freq*time + phase/M_PI, 1);
      break;
  }
  return sample < threshold_low
    ? threshold_low
    : (sample > threshold_high
       ? threshold_high
       : sample);
}

/*
double Synthesizer::_Operator::calculateSample(double time, double duration)
{
  const double phase = _calculatePhase(time, duration);
  double sample = 0;
  
  if (pitch_glide())
  {
    static double chromatic_ratio = pow(2, 1/12.0);
    
    const int divisions_per_second = 100;
    const int divisions = ceil(divisions_per_second * duration) + 1;
    const auto tone_interval = _toneInterval();
    const double interp = time * divisions_per_second;
    
    for (int j = 0; j < 2; j++)
    {
      const double p = (int)(j + interp) / (double)divisions;
      const double tone = tone_interval.first*(1-p) + tone_interval.second*p;
      const double freq = 440*pow(chromatic_ratio, tone);
      const double amplitude = j == 0 ? 1-fmod(interp, 1.0) : fmod(interp, 1.0);
      
      switch (wave_type)
      {
        case SMOOTH:
          sample += amplitude*sin(2 * M_PI * freq * time + phase);
          break;
        case SQUARE:
          sample += amplitude*(2*((int)(freq*2*time + phase/M_PI) % 2)-1);
          break;
        case TRIANGLE:
          sample += amplitude*(2*fabs(1-2*fmod(freq*time + phase/M_PI, 1))-1);
          break;
        case SAWTOOTH:
          sample += amplitude*(1-2*fmod(freq*time + phase/M_PI, 1));
          break;
      }
    }
  }
  else
  {
    switch (wave_type)
    {
      case SMOOTH:
        sample = sin(2 * M_PI * frequency * time + phase);
        break;
      case SQUARE:
        sample = 2*((int)(frequency*2*time + phase/M_PI) % 2)-1;
        break;
      case TRIANGLE:
        sample = 2*fabs(1-2*fmod(frequency*time + phase/M_PI, 1))-1;
        break;
      case SAWTOOTH:
        sample = 1-2*fmod(frequency*time + phase/M_PI, 1);
        break;
    }
  }
  
  return sample < threshold_low
  ? threshold_low
  : (sample > threshold_high
     ? threshold_high
     : sample);
}
*/

/*
double Synthesizer::_Operator::_amplitude(double time,
                                          double midpoint,
                                          double duration)
{
  time -= midpoint;
  
  if (time >= -duration/2 && time < 0.0)     return -2*time/duration;
  else if (time >= 0.0 && time < duration/2) return 1-2*time/duration;
  
  return 0.0;
};
*/

/*
double _amplitude2(double time, double midpoint, int divisions)
{
  const double low = midpoint-0.5;
  const double high = midpoint+0.5;
  time = (time < low ? low : (time > high ? high : time));
  return (cos(2*M_PI*(time-midpoint)) + 1) / (2*divisions);
};
 */

double Synthesizer::_Operator::_calculatePhase(double time, double duration)
{
  double modulated_phase = 0.0;
  for (auto modulator : modulators)
  {
    double modulator_sample = modulator->calculateSample(time, duration);
    modulator_sample       *= modulator->modulation_index;
    modulated_phase        += modulator_sample;
  }
  return modulated_phase;
}

pair<double, double> Synthesizer::_Operator::_toneInterval()
{
  static const double log_440_hz          = log(440);
  static const double log_chromatic_ratio = log(2) / 12.0;
  static const double k                   = log_440_hz / log_chromatic_ratio;
  
  if (_prev_freq != frequency)
  {
    _log_freq = log(frequency);
    _tone_dist = _log_freq/log_chromatic_ratio - k;
    _prev_freq = frequency;
  }
  
  if (_prev_gliss_freq != *pitch_glide())
  {
    _prev_gliss_freq = *pitch_glide();
    _log_gliss_freq = log(_prev_gliss_freq);
    _gliss_tone_dist = _log_gliss_freq/log_chromatic_ratio - k;
  }
  
  return {_tone_dist, _gliss_tone_dist};
}

Synthesizer::Synthesizer(int bit_rate, int sample_rate)
  : bit_rate(bit_rate)
  , sample_rate(sample_rate)
  , _algorithms()
  , _current_algorithm(nullptr)
{}

void Synthesizer::load(const char * filename)
{
  // reset synthesizer properties
  string id = filename;
#ifdef __APPLE__
  long begin = id.rfind("/")+1;
#elif defined(_WIN32)
  long begin = id.rfind("\\")+1;
#endif
  long length = id.rfind(".") - begin;
  id = id.substr(begin, length);
  _Algorithm & algorithm = _algorithms[id];
  algorithm.operators.clear();
  algorithm.num_carriers = 0;
  
  // parse xml file
  XMLDocument document;
  if (document.LoadFile(filename))
  {
    document.PrintError();
    return;
  };
  
  /*---- count_operators ----*/
  function<int(XMLElement*)> count_operators;
  count_operators = [&count_operators](XMLElement * element)
  {
    if (element && strcmp(element->Name(), "operator") == 0)
    {
      return 1 +
        count_operators(element->NextSiblingElement()) +
        count_operators(element->FirstChildElement());
    }
    return 0;
  };
  
  /*---- build_algorithm ----*/
  function<void(XMLElement*, bool, _Operator*, int&)> build_algorithm;
  build_algorithm = [this, &build_algorithm, &algorithm](XMLElement * element,
                                                         bool is_carrier,
                                                         _Operator * parent,
                                                         int & index)
  {
    // set reference to current operator to null
    _Operator * op_ptr = nullptr;
    
    // check if element is an operator
    if (strcmp(element->Name(), "operator") == 0)
    {
      if (is_carrier) algorithm.num_carriers++;
     
      // add current operator as modulator to parent operator
      op_ptr = &algorithm.operators[index++];
      _Operator & op = *op_ptr;
      if (parent) parent->addModulator(op_ptr);
      
      //// query operator attributes
      // frequency
      element->QueryDoubleAttribute("frequency", &op.frequency);
      op.frequency = max(op.frequency, 0.0);
      
      // modulation index
      element->QueryDoubleAttribute("modulation_index", &op.modulation_index);
      op.modulation_index = max(op.modulation_index, 0.0);
      
      // threshold low
      element->QueryDoubleAttribute("threshold_low", &op.threshold_low);
      op.threshold_low = op.threshold_low < -1.0
        ? -1.0
        : (op.threshold_low > 1.0
          ? 1.0
          : op.threshold_low);
      
      // threshold high
      element->QueryDoubleAttribute("threshold_high", &op.threshold_high);
      op.threshold_high = op.threshold_high < -1.0
      ? -1.0
      : (op.threshold_high > 1.0
        ? 1.0
        : op.threshold_high);
      
      // wave type
      const char * wave_type;
      if ((wave_type = element->Attribute("wave_type")))
      {
        if      (strcmp(wave_type, "SMOOTH")   == 0) op.wave_type = SMOOTH;
        else if (strcmp(wave_type, "TRIANGLE") == 0) op.wave_type = TRIANGLE;
        else if (strcmp(wave_type, "SAWTOOTH") == 0) op.wave_type = SAWTOOTH;
        else if (strcmp(wave_type, "SQUARE")   == 0) op.wave_type = SQUARE;
      }
      
      // glissando frequency
      double pitch_glide;
      if (!element->QueryDoubleAttribute("pitch_glide",
                                         &pitch_glide))
      {
        pitch_glide = max(pitch_glide, 0.0);
        op.pitch_glide = maybe<double>::just(pitch_glide);
      }
    }
    
    // recurse on siblings
    auto sibling = element->NextSiblingElement();
    if (sibling) build_algorithm(sibling, is_carrier, parent, index);
    
    // recurse on children
    auto first_child = element->FirstChildElement();
    if (first_child) build_algorithm(first_child, false, op_ptr, index);
  };
  
  // build algorithm
  XMLElement * root = document.FirstChildElement();
  if (root)
  {
    algorithm.operators = vector<_Operator>(count_operators(root));
    int index = 0;
    build_algorithm(root, true, nullptr, index);
  }
}


void Synthesizer::select(string id)
{
  if (_algorithms.find(id) != _algorithms.end())
  {
    _current_algorithm = &_algorithms[id];
  }
}

bool Synthesizer::generate(int16_t * stream,
                           int length,
                           int & frame,
                           double max_volume,
                           double duration,
                           double fade_in,
                           double fade_out)
{
  static const int max_amplitude = pow(2, bit_rate()-1)-1;
  static const int scale         = pow(2, 16-bit_rate());
  
  if (_current_algorithm)
  {
    _Algorithm & algorithm = *_current_algorithm;
    int16_t sample = 0;
    for (int i = 0; i < length; i++)
    {
      double time = frame / (double)sample_rate();
      if (time < duration)
      {
        // calculate waveform
        double waveform = 0;
        for (auto i = 0; i < algorithm.num_carriers; i++)
        {
          waveform += algorithm.operators[i].calculateSample(time, duration);
        }
        
        // calculate fading volume
        double fading_volume = min(time/fade_in, 1.0) *
                               min((duration-time)/fade_out, 1.0);
        
        // resize waveform to the maximum amplitude for the current bit rate
        sample = round(max_amplitude * waveform);
        
        // scale to 16 bit space and apply volumes
        sample *= scale * fading_volume * max_volume;
        
        // write to stream
        stream[i] += sample;
      }
      else
      {
        frame = 0;
        return true;
      }
      
      // update frame number
      frame++;
    }
    return false;
  }
  return true;
}


//
// MARK: - AudioComponent
//

string AudioComponent::trait()
{
  return "audio";
}

void AudioComponent::init(Entity * entity)
{
  Component::init(entity);
  
  synthesizer().sample_rate = entity->core()->sample_rate();
  _audio_playback = vector<_Audio>();
}

void AudioComponent::playSound(string id,
                               double duration,
                               double fade_in,
                               double fade_out)
{
  _audio_playback.push_back({id, duration, fade_in, fade_out, 0});
}

void AudioComponent::audioStreamCallback(double max_volume,
                                         int16_t * stream,
                                         int length)
{
  for (auto i = (int)_audio_playback.size() - 1; i >= 0; i--)
  {
    _Audio & audio = _audio_playback[i];
    synthesizer().select(audio.id);
    bool completed = synthesizer().generate(stream,
                                            length,
                                            audio.frame,
                                            max_volume,
                                            audio.duration,
                                            audio.fade_in,
                                            audio.fade_out);
    if (completed) _audio_playback.erase(_audio_playback.begin() + i);
  }
}
