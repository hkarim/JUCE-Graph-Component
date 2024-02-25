#pragma once

#include "JuceHeader.h"


struct PianoRollTheme {
  static constexpr unsigned int whiteKeysBg = 0xff2a2d31;
  static constexpr unsigned int blackKeysBg = 0xff232528;

  static constexpr unsigned int vBarLineFg = 0xff44484c;
  static constexpr unsigned int vQuantizeLineFg = 0xff34373a;
  static constexpr unsigned int hWhiteLanesSeparatorFg = 0xff1f2123;
  static constexpr unsigned int hOctaveLanesSeparatorFg = 0xff3a3d42;
  static constexpr int hWhiteLanesSeparatorHeight = 1;

};

struct NoteGridComponent : juce::Component {

  int laneHeight{8};
  NoteGridComponent(): juce::Component() {

  }

  void paint(juce::Graphics &g) override {
    auto bounds = getLocalBounds();
    auto w = bounds.getWidth();
    //auto h = bounds.getHeight();

    // c   d   e f   g   a   b
    // w   w   w w   w   w   w
    // 0 1 2 3 4 5 6 7 8 9 1011
    auto cWhiteKeysBg = juce::Colour(PianoRollTheme::whiteKeysBg);
    auto cBlackKeysBg = juce::Colour(PianoRollTheme::blackKeysBg);
    auto cWhiteLanesLineFg = juce::Colour(PianoRollTheme::hWhiteLanesSeparatorFg);
    auto cOctaveLanesSeparatorFg = juce::Colour(PianoRollTheme::hOctaveLanesSeparatorFg);

    // draw white and black lanes
    // top: G8 127,  bottom: C-2 0
    auto i = 0;
    auto x = 0;
    while (i <= 127) {
      auto n = 127 - i; // the note number
      auto r = n % 12;
      auto white = r == 0 || r == 2 || r == 4 || r == 5 || r == 7 || r == 9 || r == 11;

      // choose the lane colour
      if (white)
        g.setColour(cWhiteKeysBg);
      else
        g.setColour(cBlackKeysBg);

      auto y = i *laneHeight;
      // fill the lane
      g.fillRect(x, y, w, laneHeight);

      // if we have 2 consecutive white lanes, draw a separator on top of them, at the bottom of the first
      // note that we draw the notes in reverse order, top to bottom
      if (r == 0 || r == 5) {
        ++i;
        y = i * laneHeight;
        // draw the separator at the bottom of the first shifting it up by the separator height
        // the colour differs whether we're separating octaves or just 2 white keys
        auto c = r == 0 ?  cOctaveLanesSeparatorFg : cWhiteLanesLineFg;
        g.setColour(c);
        g.fillRect(x, y - PianoRollTheme::hWhiteLanesSeparatorHeight, w, PianoRollTheme::hWhiteLanesSeparatorHeight);

        // draw the consecutive white lane
        g.setColour(cWhiteKeysBg);
        g.fillRect(x, y, w, laneHeight);
      }

      // if we are between 2 white lanes, draw a darker line to separate them
//      auto whiteSeparator = r == 1 || r == 7;
//      if (whiteSeparator) {
//        g.setColour(cWhiteLanesLineFg);
//        g.fillRect(x, y + PianoRollTheme::laneHeight, w, PianoRollTheme::hWhiteLanesSeparatorHeight);
//      }

      ++i;
    }
  }

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NoteGridComponent)
};
