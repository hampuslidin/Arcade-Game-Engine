//
//  audio.cpp
//  Arcade Game Engine
//

#include <algorithm>
#include "tinyxml2.h"
#include "core.hpp"

using namespace tinyxml2;

//
// MARK: - Synthesizer
//

Synthesizer::_Operator::_Operator(double frequency,
                                  double modulation_index,
                                  WaveType wave_type,
                                  double threshold_low,
                                  double threshold_high,
                                  maybe<double> pitch_glide,
                                  PitchGlideType pitch_glide_type)
  : frequency(frequency)
  , modulation_index(modulation_index)
  , wave_type(wave_type)
  , threshold_low(threshold_low)
  , threshold_high(threshold_high)
  , modulators()
  , pitch_glide(pitch_glide)
  , pitch_glide_type(pitch_glide_type)
{}

void Synthesizer::_Operator::addModulator(_Operator * modulator)
{
  modulators.push_back(modulator);
}

double Synthesizer::_Operator::calculateSample(double time, double duration)
{
  static const double two_pi = 2*M_PI;
  double phase;
  if (!pitch_glide.isNothing() && pitch_glide != frequency)
  {
    double f0 = frequency;
    double f1 = pitch_glide;
    double k;
    switch (pitch_glide_type)
    {
      case LINEAR:
        // linear chirp
        k = (f1-f0)/duration;
        phase = two_pi*(f0*time + k/2*pow(time, 2));
        break;
      case EXPONENTIAL:
        // exponential chirp
        k = pow(f1/f0, 1/duration);
        phase = two_pi*f0*((pow(k, time)-1)/log(k));
        break;
      case INV_LOGARITHMIC:
        // inverse logarithmic chirp
        time = duration-time;
        f0 = pitch_glide;
        f1 = frequency;
      case LOGARITHMIC:
        // logarithmic chirp
        bool reverse = f1 < f0;
        k = log(reverse ? (f0-f1) : (f1-f0))/duration;
        phase = two_pi*(f0*time + (reverse ? -1 : 1)*(exp(k * time)-1)/k);
        break;
    }
  }
  else
  {
    phase = two_pi * frequency * time;
  }
  
  phase += _calculatePhase(time, duration);
  double sample;
  
  switch (wave_type)
  {
    case SMOOTH:
      sample = sin(phase);
      break;
    case TRIANGLE:
      sample = 2*fabs(2*fmod(phase/two_pi+0.75, 1)-1)-1;
      break;
    case SAWTOOTH:
      sample = 2*fmod(phase/two_pi+0.5, 1)-1;
      break;
    case SQUARE:
      sample = 2*((int)(2*phase/two_pi+1) % 2)-1;
      break;
  }
  return sample < threshold_low
    ? threshold_low
    : (sample > threshold_high
       ? threshold_high
       : sample);
}

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

Synthesizer::Synthesizer(int bit_rate, int sample_rate)
  : bit_rate(bit_rate)
  , sample_rate(sample_rate)
  , _algorithms()
  , _current_algorithm(nullptr)
{}

void Synthesizer::load(const char * filename)
{
  
  // generate id
  string id = filename;
#ifdef __APPLE__
  long begin = (long)(id.rfind("/")+1);
#elif defined(_WIN32)
  long begin = (long)(id.rfind("\\")+1);
#endif
  long length = (long)(id.rfind(".") - begin);
  id = id.substr(begin, length);
  
  // reset synthesizer properties
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
      op.frequency = std::max(op.frequency, 0.0);
      
      // modulation index
      element->QueryDoubleAttribute("modulation_index", &op.modulation_index);
      op.modulation_index = std::max(op.modulation_index, 0.0);
      
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
      
      // pitch glide
      double pitch_glide;
      if (!element->QueryDoubleAttribute("pitch_glide",
                                         &pitch_glide))
      {
        pitch_glide = std::max(pitch_glide, 0.0);
        op.pitch_glide = maybe<double>::just(pitch_glide);
      }
      
      // pitch glide type
      const char * pitch_glide_type;
      if ((pitch_glide_type = element->Attribute("pitch_glide_type")))
      {
        if (strcmp(pitch_glide_type, "LINEAR") == 0)
        {
          op.pitch_glide_type = LINEAR;
        }
        else if (strcmp(pitch_glide_type, "EXPONENTIAL") == 0)
        {
          op.pitch_glide_type = EXPONENTIAL;
        }
        else if (strcmp(pitch_glide_type, "LOGARITHMIC") == 0)
        {
          op.pitch_glide_type = LOGARITHMIC;
        }
        else if (strcmp(pitch_glide_type, "INV_LOGARITHMIC") == 0)
        {
          op.pitch_glide_type = INV_LOGARITHMIC;
        }
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
  static const int max_amplitude = (int)(pow(2, bit_rate()-1)-1);
  static const int scale         = (int)pow(2, 16-bit_rate());
  
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
        waveform /= algorithm.num_carriers;
        
        // calculate fading volume
        double fading_volume = std::min(time/fade_in, 1.0) *
                               std::min((duration-time)/fade_out, 1.0);
        
        // resize waveform to the maximum amplitude for the current bit rate
        sample = (int16_t)round(max_amplitude * waveform);
        
        // scale to 16 bit space and apply volumes
        sample *= (int16_t)(scale * fading_volume * max_volume);
        
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
  return "Audio";
}

void AudioComponent::init(Entity * entity)
{
  Component::init(entity);
  
  synthesizer().sample_rate = entity->core()->sampleRate();
  _audio_playback = vector<_Audio>();
}

void AudioComponent::playSound(string id,
                               double duration,
                               double fade_in,
                               double fade_out)
{
  _audio_playback.push_back({id, duration, fade_in, fade_out, 0});
}

void AudioComponent::_audioStreamCallback(double max_volume,
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
