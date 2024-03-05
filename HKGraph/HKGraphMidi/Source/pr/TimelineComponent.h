#pragma once

#include "JuceHeader.h"
#include "PianoRollTheme.h"
#include "Measure.h"

struct TimelineComponent : public juce::Component {
  Measure::FloatTimeSignature timeSignature{};
  float playHeadPosition{0.0f};
  int bars{32};
  float unit{0.0f};
  int quantize{2};

  std::function<void(const juce::MouseEvent &)> onMouseDown;

  explicit TimelineComponent(
    Measure::FloatTimeSignature ts,
    int numberOfBars,
    float gridUnit,
    int quantization)
    : juce::Component(),
      timeSignature(ts),
      bars(numberOfBars),
      unit(gridUnit),
      quantize(quantization) {}

  ~TimelineComponent() override = default;

  void paint(juce::Graphics &g) override {
    auto bounds = getLocalBounds();
    g.setColour(juce::Colour(0xff343d48));
    g.fillRect(bounds);

    // draw bar lines
    auto vBarSeparatorWidth = PianoRollTheme::vBarSeparatorWidth / scaledWidth * 0.5f;
    g.setColour(juce::Colours::white);
    auto f = g.getCurrentFont();
    f.setHeight(8.0f);
    g.setFont(f);
    auto x = 0.0f;
    auto unitsPerBar = (static_cast<int>(timeSignature.numerator) * 64) / static_cast<int>(timeSignature.denominator);
    auto beats = unitsPerBar / static_cast<int>(timeSignature.numerator);
    std::cout << "unitsPerBar = " << unitsPerBar << ", quantize = " << quantize << ", unit = " << unit << std::endl;
    for (auto bar = 0; bar < bars; ++bar) {
      for (auto q = 0; q < unitsPerBar; ++q) {
        auto onBar = q == 0;
        auto onBeat = q % beats == 0;
        if (onBeat && !onBar)
          g.setColour(juce::Colours::black);
        else
          g.setColour(juce::Colours::white);
        g.fillRect(
          static_cast<float>(x),
          onBar ? 10.0f : 20.0f,
          vBarSeparatorWidth,
          static_cast<float>(bounds.getHeight()));
        x += unit;
      }
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