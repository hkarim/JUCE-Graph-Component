#pragma once

#include "JuceHeader.h"

struct Measure {
  static int beatsPerBar(const juce::AudioPlayHead::TimeSignature &ts) {
    return ts.numerator;
  }

  static int ticksPerBeat(int quantize) {
    return std::min(quantize, 16);
  }

  static int ticksPerBar(const juce::AudioPlayHead::TimeSignature &ts, int quantize) {
    return beatsPerBar(ts) * ticksPerBeat(quantize);
  }

  static int beatWidth(int tickWidth) {
    return tickWidth * 16;
  }

  static int barWidth(int tickWidth, const juce::AudioPlayHead::TimeSignature &ts) {
    return beatWidth(tickWidth) * beatsPerBar(ts);
  }

  static int minimumNoteWidth(int tickWidth, int quantize) {
    return ticksPerBeat(quantize) * tickWidth;
  }

  static int quantizedTickWidth(int tickWidth, int quantize) {
    return (16 / ticksPerBeat(quantize)) * tickWidth;
  }
};
