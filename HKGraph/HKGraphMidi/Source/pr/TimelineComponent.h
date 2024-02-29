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
    auto vBarSeparatorWidth = PianoRollTheme::vBarSeparatorWidth / scaledWidth;
    auto i = 0;
    auto x = 0;
    auto y = 0;
    g.setColour(juce::Colours::white);
    while (i <= bars) {
      x = i * barWidth;
      g.fillRect(
        static_cast<float>(x),
        static_cast<float>(y),
        vBarSeparatorWidth,
        static_cast<float>(bounds.getHeight()));
      ++i;
      // draw quantize bar lines
      auto sub = barWidth / timeSignature.numerator;
      for (auto j = 1; j < barWidth; j += sub) {
        x += sub;
        g.fillRect(
          static_cast<float>(x),
          static_cast<float>(y),
          vBarSeparatorWidth,
          static_cast<float>(bounds.getHeight()));
      }
    }
    g.setColour(juce::Colours::black);
    // line below
    g.fillRect(0, bounds.getHeight() - 2, bounds.getWidth(), 2);
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