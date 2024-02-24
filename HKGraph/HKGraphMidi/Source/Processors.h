#pragma once
#include <string>
#include "JuceHeader.h"

struct Processors {
  static const std::string midiInNodeProcessor;
  static const std::string midiOutNodeProcessor;
  static const std::string passthroughProcessor;
  static const std::string channelRouterProcessor;
  static const std::string channelSplitterProcessor;
  static const std::string chordSplitterProcessor;
  static const std::string keyboardProcessor;
  static const std::string noteFilterProcessor;
  static const std::string transposeProcessor;
  static const std::string velocityCurveProcessor;
  static const std::string controllerCurveProcessor;
};

struct Block {
  juce::AudioBuffer<float> audioBuffer;
  juce::MidiBuffer midiBuffer;
};
