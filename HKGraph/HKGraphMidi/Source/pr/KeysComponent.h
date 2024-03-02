#pragma once

#include "JuceHeader.h"
#include "PianoRollTheme.h"

struct KeysComponent : juce::Component {

  int laneHeight{8};
  int nKeys{128};

  KeysComponent(int gridLaneHeight, int numberOfKeys)
    : juce::Component(), laneHeight(gridLaneHeight), nKeys(numberOfKeys) {
    setSize(32, nKeys * laneHeight);
  }

  ~KeysComponent() override = default;

  void paint(juce::Graphics &g) override {
    auto bounds = getLocalBounds();
    g.setColour(juce::Colour(0xff343d48));
    g.fillRect(bounds);

    auto w = bounds.getWidth();
    auto x = 0;
    int y;
    auto whiteKeyHeight = (12 * laneHeight) / 7;
    auto blackKeyWidth = w - 24;
    auto i = 0;
    int yw{i * laneHeight}; // white key y position
    auto zeroBasedKeys = nKeys - 1;
    while (i <= zeroBasedKeys) {
      auto n = zeroBasedKeys - i; // the note number
      auto r = n % 12;
      auto white = r == 0 || r == 2 || r == 4 || r == 5 || r == 7 || r == 9 || r == 11;
      auto kw = w;
      auto kh = laneHeight;
      // choose the lane colour
      if (white) {
        g.setColour(juce::Colours::white);
        if (i == 0)
          // this is the first note (G8) from the top
          // we need to set its height so that other white keys are correctly aligned
          // the total height of the current octave (C8:G8) is 8 * laneHeight
          // excluding G8, we need 4 white keys 4 * whiteKeyHeight
          // then G8 should have a height of (8 * laneHeight) - (4 * whiteKeyHeight)
          kh = (8 * laneHeight) - (4 * whiteKeyHeight);
        else
          kh = whiteKeyHeight;
        // fill the white key
        g.fillRect(x, yw, kw, kh);
        // draw border
        g.setColour(juce::Colours::grey);
        auto border = juce::Rectangle(x, yw, kw, kh).toFloat();
        g.drawRect(border, 0.3f / scaledWidth);
        yw += kh;
      }
      ++i;
    }
    i = 0;
    while (i <= zeroBasedKeys) {
      auto n = zeroBasedKeys - i; // the note number
      auto r = n % 12;
      auto white = r == 0 || r == 2 || r == 4 || r == 5 || r == 7 || r == 9 || r == 11;
      y = i * laneHeight;
      if (!white) {
        g.setColour(juce::Colours::black);
        // fill the black key
        g.fillRect(x, y, blackKeyWidth, laneHeight);
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
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KeysComponent)
};