#pragma once

#include "JuceHeader.h"

struct Measure {

  static int beatsPerBar(const juce::AudioPlayHead::TimeSignature &ts) {
    return ts.numerator;
  }

  static int unitsPerBar(const juce::AudioPlayHead::TimeSignature &ts) {
    return (ts.numerator * 64) / ts.denominator;
  }

  static int unitsPerBeat(const juce::AudioPlayHead::TimeSignature &ts) {
    return unitsPerBar(ts) / beatsPerBar(ts);
  }

  static int unitsPerQuantize(const juce::AudioPlayHead::TimeSignature &ts, int quantize) {
    auto q = quantize;
    auto upb = unitsPerBar(ts);
    auto upe = unitsPerBeat(ts);
    while (upb % q != 0) {
      q = q / 2;
    }
    auto result = upb / q;
    while (upe % result != 0) {
      result -= 1;
    }
    if (result <= 0) {
      result = upb;
    }
    return result;
  }

  static int barWidth(const juce::AudioPlayHead::TimeSignature &ts, int unit) {
    return unitsPerBar(ts) * unit;
  }

  static int beatWidth(const juce::AudioPlayHead::TimeSignature &ts, int unit) {
    return unitsPerBeat(ts) * unit;
  }

  static int quantizeWidth(const juce::AudioPlayHead::TimeSignature &ts, int quantize, int unit) {
    return unitsPerQuantize(ts, quantize) * unit;
  }

};
