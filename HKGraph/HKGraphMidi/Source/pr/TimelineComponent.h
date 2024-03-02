#pragma once

#include "JuceHeader.h"
#include "PianoRollTheme.h"

struct TimelineComponent : public juce::Component {
  juce::AudioPlayHead::TimeSignature timeSignature{};
  float playHeadPosition{0.0f};
  int bars{32};
  int barWidth{64};
  int quantize{2};

  std::function<void(const juce::MouseEvent &)> onMouseDown;

  explicit TimelineComponent(
    juce::AudioPlayHead::TimeSignature ts,
    int numberOfBars,
    int gridBarWidth,
    int quantization)
    : juce::Component(),
      timeSignature(ts),
      bars(numberOfBars),
      barWidth(gridBarWidth),
      quantize(quantization) {}

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
    auto sub = PianoRollTheme::calculateTicks(barWidth, quantize, timeSignature);
    std::cout << "[Timeline::sub] " << sub << std::endl;
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

    // draw play head
    g.setColour(juce::Colours::white);
    g.fillRect(
      playHeadPosition,
      0.0f,
      vBarSeparatorWidth,
      static_cast<float>(bounds.getHeight())
    );
  }

  void mouseDown(const juce::MouseEvent &e) override {
    if (onMouseDown != nullptr)
      onMouseDown(e);
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