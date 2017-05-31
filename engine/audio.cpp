//
//  audio.cpp
//  Arcade Game Engine
//

#include <tinyxml2.h>

#include "core.hpp"

using namespace tinyxml2;


// MARK: Properties

int Synthesizer::bitRate() const    { return _bitRate; }
int Synthesizer::sampleRate() const { return _sampleRate; }

// MARK: Member functions

Synthesizer::Synthesizer(int bitRate, int sampleRate)
  : _bitRate(bitRate)
  , _sampleRate(sampleRate)
  , _algorithms()
  , _currentAlgorithm(nullptr)
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
  algorithm.numCarriers = 0;
  
  // parse xml file
  XMLDocument document;
  if (document.LoadFile(filename))
  {
    document.PrintError();
    return;
  };
  
  /*---- countOperators ----*/
  function<int(XMLElement*)> countOperators;
  countOperators = [&countOperators](XMLElement * element)
  {
    if (element && strcmp(element->Name(), "operator") == 0)
      return 1 +
        countOperators(element->NextSiblingElement()) +
        countOperators(element->FirstChildElement());
    return 0;
  };
  
  /*---- buildAlgorithm ----*/
  function<void(XMLElement*, bool, _Operator*, int&)> buildAlgorithm;
  buildAlgorithm = [this, &buildAlgorithm, &algorithm](XMLElement * element,
                                                         bool isCarrier,
                                                         _Operator * parent,
                                                         int & index)
  {
    // set reference to current operator to null
    _Operator * opPtr = nullptr;
    
    // check if element is an operator
    if (strcmp(element->Name(), "operator") == 0)
    {
      if (isCarrier) algorithm.numCarriers++;
     
      // add current operator as modulator to parent operator
      opPtr = &algorithm.operators[index++];
      _Operator & op = *opPtr;
      if (parent) parent->modulators.push_back(opPtr);
      
      //// query operator attributes
      // frequency
      element->QueryDoubleAttribute("frequency", &op.frequency);
      op.frequency = std::max(op.frequency, 0.0);
      
      // modulation index
      element->QueryDoubleAttribute("modulation_index", &op.modulationIndex);
      op.modulationIndex = std::max(op.modulationIndex, 0.0);
      
      // threshold low
      element->QueryDoubleAttribute("threshold_low", &op.thresholdLow);
      op.thresholdLow = op.thresholdLow < -1.0
        ? -1.0
        : (op.thresholdLow > 1.0
          ? 1.0
          : op.thresholdLow);
      
      // threshold high
      element->QueryDoubleAttribute("threshold_high", &op.thresholdHigh);
      op.thresholdHigh = op.thresholdHigh < -1.0
      ? -1.0
      : (op.thresholdHigh > 1.0
        ? 1.0
        : op.thresholdHigh);
      
      // wave type
      const char * waveType;
      if ((waveType = element->Attribute("wave_type")))
      {
        if      (strcmp(waveType, "SMOOTH")   == 0) op.waveType = SMOOTH;
        else if (strcmp(waveType, "TRIANGLE") == 0) op.waveType = TRIANGLE;
        else if (strcmp(waveType, "SAWTOOTH") == 0) op.waveType = SAWTOOTH;
        else if (strcmp(waveType, "SQUARE")   == 0) op.waveType = SQUARE;
      }
      
      // pitch glide
      double pitchGlide;
      if (!element->QueryDoubleAttribute("pitch_glide", &pitchGlide))
      {
        pitchGlide = std::max(pitchGlide, 0.0);
        op.pitchGlide = maybe<double>::just(pitchGlide);
      }
      
      // pitch glide type
      const char * pitchGlideType;
      if ((pitchGlideType = element->Attribute("pitch_glide_type")))
      {
        if (strcmp(pitchGlideType, "LINEAR") == 0)
          op.pitchGlideType = LINEAR;
        else if (strcmp(pitchGlideType, "EXPONENTIAL") == 0)
          op.pitchGlideType = EXPONENTIAL;
        else if (strcmp(pitchGlideType, "LOGARITHMIC") == 0)
          op.pitchGlideType = LOGARITHMIC;
        else if (strcmp(pitchGlideType, "INV_LOGARITHMIC") == 0)
          op.pitchGlideType = INV_LOGARITHMIC;
      }
    }
    
    // recurse on siblings
    auto sibling = element->NextSiblingElement();
    if (sibling) buildAlgorithm(sibling, isCarrier, parent, index);
    
    // recurse on children
    auto firstChild = element->FirstChildElement();
    if (firstChild) buildAlgorithm(firstChild, false, opPtr, index);
  };
  
  // build algorithm
  XMLElement * root = document.FirstChildElement();
  if (root)
  {
    algorithm.operators = vector<_Operator>(countOperators(root));
    int index = 0;
    buildAlgorithm(root, true, nullptr, index);
  }
}


void Synthesizer::select(string id)
{
  if (_algorithms.find(id) != _algorithms.end())
    _currentAlgorithm = &_algorithms[id];
}

bool Synthesizer::generate(int16_t * stream,
                           int length,
                           int & frame,
                           double maxVolume,
                           double duration,
                           double fadeIn,
                           double fadeOut)
{
  static const int maxAmplitude = (int)(pow(2, _bitRate-1)-1);
  static const int scale         = (int)pow(2, 16-_bitRate);
  
  if (_currentAlgorithm)
  {
    _Algorithm & algorithm = *_currentAlgorithm;
    int16_t sample = 0;
    for (int i = 0; i < length; i++)
    {
      double time = frame / (double)_sampleRate;
      if (time < duration)
      {
        // calculate waveform
        double waveform = 0;
        for (auto i = 0; i < algorithm.numCarriers; i++)
        {
          waveform += _calculateSample(algorithm.operators[i], time, duration);
        }
        waveform /= algorithm.numCarriers;
        
        // calculate fading volume
        double fadingVolume = std::min(time/fadeIn, 1.0) *
                              std::min((duration-time)/fadeOut, 1.0);
        
        // resize waveform to the maximum amplitude for the current bit rate
        sample = (int16_t)round(maxAmplitude * waveform);
        
        // scale to 16 bit space and apply volumes
        sample *= (int16_t)(scale * fadingVolume * maxVolume);
        
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

// MARK: Private
double Synthesizer::_calculateSample(_Operator op, double time, double duration)
{
  static const double twoPi = 2*M_PI;
  double phase;
  if (!op.pitchGlide.isNothing() && op.pitchGlide != op.frequency)
  {
    double f0 = op.frequency;
    double f1 = op.pitchGlide;
    double k;
    switch (op.pitchGlideType)
    {
      case LINEAR:
        // linear chirp
        k = (f1-f0)/duration;
        phase = twoPi*(f0*time + k/2*pow(time, 2));
        break;
      case EXPONENTIAL:
        // exponential chirp
        k = pow(f1/f0, 1/duration);
        phase = twoPi*f0*((pow(k, time)-1)/log(k));
        break;
      case INV_LOGARITHMIC:
        // inverse logarithmic chirp
        time = duration-time;
        f0 = op.pitchGlide;
        f1 = op.frequency;
      case LOGARITHMIC:
        // logarithmic chirp
        bool reverse = f1 < f0;
        k = log(reverse ? (f0-f1) : (f1-f0))/duration;
        phase = twoPi*(f0*time + (reverse ? -1 : 1)*(exp(k * time)-1)/k);
        break;
    }
  }
  else
  {
    phase = twoPi * op.frequency * time;
  }
  
  phase += _calculatePhase(op, time, duration);
  double sample;
  
  switch (op.waveType)
  {
    case SMOOTH:
      sample = sin(phase);
      break;
    case TRIANGLE:
      sample = 2*fabs(2*fmod(phase/twoPi+0.75, 1)-1)-1;
      break;
    case SAWTOOTH:
      sample = 2*fmod(phase/twoPi+0.5, 1)-1;
      break;
    case SQUARE:
      sample = 2*((int)(2*phase/twoPi+1) % 2)-1;
      break;
  }
  return sample < op.thresholdLow
    ? op.thresholdLow
    : (sample > op.thresholdHigh
      ? op.thresholdHigh
      : sample);
}

double Synthesizer::_calculatePhase(_Operator op, double time, double duration)
{
  double modulatedPhase = 0.0;
  for (auto modulator : op.modulators)
  {
    double modulatorSample = _calculateSample(*modulator, time, duration);
    modulatorSample       *= modulator->modulationIndex;
    modulatedPhase        += modulatorSample;
  }
  return modulatedPhase;
}


// MARK: -
// MARK: Properties
string AudioComponent::trait() const { return "Audio"; }

// MARK: Member functions
void AudioComponent::init(Entity * entity)
{
  Component::init(entity);
  
  _synthesizer = Synthesizer(8, entity->core()->sampleRate());
  _audioPlayback = vector<_Audio>();
}

void AudioComponent::playSound(string id,
                               double duration,
                               double fadeIn,
                               double fadeOut)
{
  _audioPlayback.push_back({id, duration, fadeIn, fadeOut, 0});
}

// MARK: Private
// TODO: refactor audio to be more consistent with the rest of the engine
void AudioComponent::_audioStreamCallback(double maxVolume,
                                          int16_t * stream,
                                          int length)
{
  for (auto i = (int)_audioPlayback.size() - 1; i >= 0; i--)
  {
    _Audio & audio = _audioPlayback[i];
    _synthesizer.select(audio.id);
    bool completed = _synthesizer.generate(stream,
                                           length,
                                           audio.frame,
                                           maxVolume,
                                           audio.duration,
                                           audio.fadeIn,
                                           audio.fadeOut);
    if (completed) _audioPlayback.erase(_audioPlayback.begin() + i);
  }
}
