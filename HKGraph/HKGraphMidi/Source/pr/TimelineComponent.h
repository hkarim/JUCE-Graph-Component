#pragma once

#include "JuceHeader.h"
#include "PianoRollTheme.h"

struct TimelineComponent : public juce::Component {
  juce::AudioPlayHead::TimeSignature timeSignature{};
  int bars{32};
  int barWidth{64};

  TimelineComponent() : juce::Component() {}

  ~TimelineComponent() override = default;

  void paint(juce::Graphics &g) override {
    auto bounds = getLocalBounds();
    g.setColour(juce::Colour(0xff343d48));
    g.fillRect(bounds);

    // draw bar lines
    auto vBarSeparatorWidth = PianoRollTheme::vBarSeparatorWidth / scaledWidth * 0.5f;
    auto i = 0;
    g.setColour(juce::Colours::white);
    while (i <= bars) {
      auto x = i * barWidth;
      // draw quantize bar lines
      auto sub = barWidth / timeSignature.numerator;
      for (auto j = 0; j < barWidth; j += sub) {
        auto alt = x % barWidth;
        auto delta = alt > 1 ? 20.0f : 10.0f;
        if (alt > 1) {
          delta = 20.0f;
        }
        g.fillRect(
          static_cast<float>(x),
          delta,
          vBarSeparatorWidth,
          static_cast<float>(bounds.getHeight()));
        x += sub;
      }
      ++i;
    }
  }

  void setScale(float widthFactor, float heightFactor) {
    scaledWidth = widthFactor;
    scaledHeight = heightFactor;
  }

private:
  float scaledWidth{1.0f};
  float scaledHeight{1.0f};
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TimelineComponent)
};