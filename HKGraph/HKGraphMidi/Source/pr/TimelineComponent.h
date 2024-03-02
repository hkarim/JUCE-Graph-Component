#pragma once

#include "JuceHeader.h"
#include "PianoRollTheme.h"
#include "Measure.h"

struct TimelineComponent : public juce::Component {
  juce::AudioPlayHead::TimeSignature timeSignature{};
  float playHeadPosition{0.0f};
  int bars{32};
  int tickWidth{8};
  int quantize{2};

  std::function<void(const juce::MouseEvent &)> onMouseDown;

  explicit TimelineComponent(
    juce::AudioPlayHead::TimeSignature ts,
    int numberOfBars,
    int gridTickWidth,
    int quantization)
    : juce::Component(),
      timeSignature(ts),
      bars(numberOfBars),
      tickWidth(gridTickWidth),
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
    auto beatsPerBar = Measure::beatsPerBar(timeSignature);
    auto beatWidth = Measure::beatWidth(tickWidth);
    for (auto bar = 0; bar < bars; ++bar) {
      // draw bar number
      g.saveState(); // we are about to set a transform
      auto at = juce::AffineTransform().scaled(1.0f / scaledWidth, 1.0f);
      g.addTransform(at);
      auto r = juce::Rectangle<float>(
        static_cast<float>(static_cast<float>(x + 2) * scaledWidth),
        4.0f,
        24.0f,
        12.0f
      );
      g.drawText(juce::String(bar + 1), r, juce::Justification::top);
      g.restoreState(); // reset transform

      // draw bar beats
      g.setColour(juce::Colours::white);
      for (auto beat = 0; beat < beatsPerBar; ++beat) {
        auto onBeat = beat % beatsPerBar == 0;
        g.fillRect(
          static_cast<float>(x),
          onBeat? 10.0f : 20.0f,
          vBarSeparatorWidth,
          static_cast<float>(bounds.getHeight()));
        // draw quantization ticks
        if (quantize > 1) {
          // quantization is one of 1, 2, 4, 8, 16
          // we have 16 ticks per beat, so maximum number of ticks is 16
          auto ticks = Measure::ticksPerBeat(quantize);
          auto tw = Measure::quantizedTickWidth(tickWidth, quantize);
          auto xt = x + tw;
          for (auto tick = 0; tick < ticks - 1 ; ++tick) {
            g.fillRect(
              static_cast<float>(xt),
              25.0f,
              vBarSeparatorWidth,
              static_cast<float>(bounds.getHeight()));
            xt += tw;
          }
        }
        x += beatWidth;
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