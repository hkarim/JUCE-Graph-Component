#pragma once

#include "JuceHeader.h"
#include "PianoRollTheme.h"

struct TimelineComponent : public juce::Component {
  juce::AudioPlayHead::TimeSignature timeSignature{};
  int bars{32};
  int barWidth{64};

  explicit TimelineComponent(juce::AudioPlayHead::TimeSignature ts, int numberOfBars, int gridBarWidth)
    : juce::Component(),
    timeSignature(ts),
    bars(numberOfBars),
    barWidth(gridBarWidth) {}

  ~TimelineComponent() override = default;

  void paint(juce::Graphics &g) override {
    auto bounds = getLocalBounds();
    g.setColour(juce::Colour(0xff343d48));
    g.fillRect(bounds);

    // draw bar lines
    auto vBarSeparatorWidth = PianoRollTheme::vBarSeparatorWidth / scaledWidth * 0.5f;
    auto i = 0;
    g.setColour(juce::Colours::white);
    auto f = g.getCurrentFont();
    f.setHeight(8.0f);
    g.setFont(f);
    while (i <= bars) {
      auto x = i * barWidth;
      // draw bar numbers
      g.saveState(); // we are about to set a transform
      auto at = juce::AffineTransform().scaled(1.0f / scaledWidth, 1.0f);
      g.addTransform(at);

      auto r = juce::Rectangle<float>(
        static_cast<float>(static_cast<float>(x) * scaledWidth),
        4.f,
        24.0f,
        12.0f
      );
      g.drawText(juce::String(i + 1), r, juce::Justification::top);
      g.restoreState(); // reset transform

      // draw quantize bar lines
      auto sub = barWidth / timeSignature.numerator;
      for (auto j = 0; j < barWidth; j += sub) {
        auto alt = x % barWidth;
        auto delta = alt > 1 ? 25.0f : 15.0f;
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