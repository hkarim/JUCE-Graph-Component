#pragma once

#include "JuceHeader.h"
#include "PianoRollTheme.h"
#include "Measure.h"

struct TimelineComponent : public juce::Component {
  juce::AudioPlayHead::TimeSignature timeSignature{};
  float playHeadPosition{0.0f};
  int bars{32};
  int unit{};
  int quantize{2};

  std::function<void(const juce::MouseEvent &)> onMouseDown;

  explicit TimelineComponent(
    juce::AudioPlayHead::TimeSignature ts,
    int numberOfBars,
    int gridUnit,
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


    auto x = 0;
    auto unitsPerBar = Measure::unitsPerBar(timeSignature);
    auto unitsPerBeat = Measure::unitsPerBeat(timeSignature);
    auto unitsPerQuantize = Measure::unitsPerQuantize(timeSignature, quantize);
    for (auto bar = 0; bar < bars; ++bar) {
      auto beat = 0;
      // draw bar numbers
      g.saveState(); // we are about to set a transform
      auto at = juce::AffineTransform().scaled(1.0f / scaledWidth, 1.0f);
      g.addTransform(at);
      auto r = juce::Rectangle<float>(
        static_cast<float>(static_cast<float>(x+unit) * scaledWidth),
        4.f,
        24.0f,
        12.0f
      );
      g.drawText(juce::String(bar + 1), r, juce::Justification::top);
      g.restoreState(); // reset transform
      for (auto u = 0; u < unitsPerBar; ++u) {
        auto onBar = u == 0;
        auto onBeat = u % unitsPerBeat == 0;
        auto onQuantize = u % unitsPerQuantize == 0;
        if (onBar) {
          g.setColour(juce::Colours::white);
          g.fillRect(
            static_cast<float>(x),
            10.0f,
            vBarSeparatorWidth,
            static_cast<float>(bounds.getHeight()));
        } else if (onBeat) {
          g.setColour(juce::Colours::white);
          // draw beat numbers
          g.saveState(); // we are about to set a transform
          g.addTransform(at);
          r = juce::Rectangle<float>(
            static_cast<float>(static_cast<float>(x) * scaledWidth),
            8.f,
            24.0f,
            12.0f
          );
          g.drawText(juce::String(bar + 1) + "." + juce::String(beat + 1), r, juce::Justification::top);
          g.restoreState(); // reset transform
          g.fillRect(
            static_cast<float>(x),
            20.0f,
            vBarSeparatorWidth,
            static_cast<float>(bounds.getHeight()));
          ++beat;
        } else if (onQuantize) {
          g.setColour(juce::Colours::whitesmoke);
          g.fillRect(
            static_cast<float>(x),
            25.0f,
            vBarSeparatorWidth,
            static_cast<float>(bounds.getHeight()));
        }
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