#pragma once
#include <string>
#include "JuceHeader.h"

struct Processors {
  static const std::string midiInNodeProcessor;
  static const std::string midiOutNodeProcessor;
  static const std::string passthroughProcessor;
  static const std::string channelRouterProcessor;
  static const std::string channelSplitterNodeProcessor;
  static const std::string keyboardProcessor;
  static const std::string transposeProcessor;
  static const std::string velocityCurveProcessor;
};

struct Block {
  juce::AudioBuffer<float> audioBuffer;
  juce::MidiBuffer midiBuffer;
};
