#pragma once

#include "JuceHeader.h"

struct Measure {
  struct FloatTimeSignature {
    float numerator{};
    float denominator{};

    static FloatTimeSignature make(juce::AudioPlayHead::TimeSignature &ts) {
      return {
        .numerator = static_cast<float>(ts.numerator),
        .denominator = static_cast<float>(ts.denominator),
      };
    }
  };

  static int beatsPerBar(const FloatTimeSignature& ts) {
    return static_cast<int>(ts.numerator);
  }

  static float unitsPerBar(const FloatTimeSignature &ts) {
    return ts.numerator / ts.denominator;
  }

  static float unitsPerBeat(const FloatTimeSignature &ts) {
    return unitsPerBar(ts) / static_cast<float>((beatsPerBar(ts)));
  }

  static float beatWidth(const FloatTimeSignature &ts, float unit) {
    return unitsPerBeat(ts) * unit;
  }

  static float barWidth(const FloatTimeSignature &ts, float unit) {
    return unitsPerBar(ts) * unit;
  }

  static float quantizeTicksPerBeat(const FloatTimeSignature &ts, int quantize) {
    return static_cast<float>(quantize) / static_cast<float>(beatsPerBar(ts));
  }

  static int quantizeTicksPerBeatInt(const FloatTimeSignature &ts, int quantize) {
    return static_cast<int>(quantizeTicksPerBeat(ts, quantize));
  }

  static float quantizeTickWidth(const FloatTimeSignature &ts, int quantize, float unit) {
    return (unitsPerBar(ts) / static_cast<float>(quantize)) * unit;
  }

};
