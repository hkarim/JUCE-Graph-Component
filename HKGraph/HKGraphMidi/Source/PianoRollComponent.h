#pragma once

#include "JuceHeader.h"


struct PianoRollTheme {
  static constexpr unsigned int whiteKeysBg = 0xff2a2d31;
  static constexpr unsigned int blackKeysBg = 0xff232528;

  static constexpr unsigned int hWhiteLanesSeparatorFg = 0xff1f2123;
  static constexpr unsigned int hOctaveLanesSeparatorFg = 0xff3a3d42;
  static constexpr unsigned int vBarSeparatorFg = 0xff3a3d42;
  static constexpr unsigned int vSubBarFg = 0xff33363a;

  static constexpr int hWhiteLanesSeparatorHeight = 1;
  static constexpr int vBarSeparatorWidth = 1;

};

struct NoteComponent : juce::Component {

  bool selected{false};

  NoteComponent() : juce::Component() {
  }

  ~NoteComponent() override = default;

  void paint(juce::Graphics &g) override {
    auto bounds = getLocalBounds();

    if (selected) {
      g.setColour(juce::Colours::orange);
    }
    else {
      g.setColour(juce::Colours::orange.darker());
    }
    g.fillRect(bounds);

    juce::Path p;
    p.addRectangle(bounds);
    g.setColour(juce::Colours::whitesmoke);
    g.strokePath(p, juce::PathStrokeType(0.4f));
  }

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NoteComponent)
};

struct NoteGridComponent : juce::Component {

  const juce::Colour cWhiteKeysBg{PianoRollTheme::whiteKeysBg};
  const juce::Colour cBlackKeysBg{PianoRollTheme::blackKeysBg};
  const juce::Colour cWhiteLanesLineFg{PianoRollTheme::hWhiteLanesSeparatorFg};
  const juce::Colour cOctaveLanesSeparatorFg{PianoRollTheme::hOctaveLanesSeparatorFg};
  const juce::Colour cBarSeparatorFg{PianoRollTheme::vBarSeparatorFg};
  const juce::Colour cSubBarFg{PianoRollTheme::vSubBarFg};

  int laneHeight{8};
  int bars{32};
  int barWidth{64};
  int quantize{0};

  std::vector<NoteComponent *> notes;

  struct ChildrenMouseListener : juce::MouseListener {

    NoteGridComponent *parent;

    explicit ChildrenMouseListener(NoteGridComponent *p) : juce::MouseListener(), parent(p) {}

    ~ChildrenMouseListener() override = default;

    void mouseDown(const juce::MouseEvent &e) override {
      if (auto note = dynamic_cast<NoteComponent *>(e.originalComponent)) {
        note->selected = true;
        note->repaint();
      }
    }

    void mouseDrag(const juce::MouseEvent &e) override {
      if (auto note = dynamic_cast<NoteComponent *>(e.originalComponent)) {
        auto minY = 0;
        auto maxY = parent->getHeight() - note->getHeight();
        auto position = note->getPosition();
        auto delta = note->getLocalPoint(note, e.getPosition());
        auto up = delta.y < 0;
        auto down = !up;
        auto right = delta.x > 0;
        auto left = !right;

        auto destination = position + delta;
        auto verticalThreshold = parent->laneHeight / 2;

        auto x = position.x;
        auto y = position.y;
        if (up && destination.y >= minY && std::abs(delta.y) >= verticalThreshold) {
          y = y - parent->laneHeight;
        }
        if (down && destination.y <= maxY && std::abs(delta.y) >= verticalThreshold) {
          y = y + parent->laneHeight;
        }

        note->setBounds(x, y, note->getWidth(), note->getHeight());
      }
    }

    void mouseUp(const juce::MouseEvent &e) override {
      juce::ignoreUnused(e);
    }

    void mouseDoubleClick(const juce::MouseEvent &e) override {
      if (auto note = dynamic_cast<NoteComponent *>(e.originalComponent)) {
        parent->removeNote(note);
      }
    }

  };

  std::unique_ptr<ChildrenMouseListener> mouseListener;

  NoteGridComponent() :
    juce::Component(),
    mouseListener(new ChildrenMouseListener(this)) {
  }

  ~NoteGridComponent() override {
    for (auto &n: notes) {
      n->removeMouseListener(mouseListener.get());
      delete n;
    }
  }


  void paint(juce::Graphics &g) override {
    auto bounds = getLocalBounds();
    auto w = bounds.getWidth();
    auto h = bounds.getHeight();

    // c   d   e f   g   a   b
    // w   w   w w   w   w   w
    // 0 1 2 3 4 5 6 7 8 9 1011


    // draw white and black lanes
    // top: G8 127,  bottom: C-2 0
    auto i = 0;
    auto x = 0;
    auto y = 0;
    while (i <= 127) {
      auto n = 127 - i; // the note number
      auto r = n % 12;
      auto white = r == 0 || r == 2 || r == 4 || r == 5 || r == 7 || r == 9 || r == 11;

      // choose the lane colour
      if (white)
        g.setColour(cWhiteKeysBg);
      else
        g.setColour(cBlackKeysBg);

      y = i * laneHeight;
      // fill the lane
      g.fillRect(x, y, w, laneHeight);

      // if we have 2 consecutive white lanes, draw a separator on top of them, at the bottom of the first
      // note that we draw the notes in reverse order, top to bottom
      if (r == 0 || r == 5) {
        ++i;
        y = i * laneHeight;
        // draw the separator at the bottom of the first shifting it up by the separator height
        // the colour differs whether we're separating octaves or just 2 white keys
        auto c = r == 0 ? cOctaveLanesSeparatorFg : cWhiteLanesLineFg;
        g.setColour(c);
        g.fillRect(x, y - PianoRollTheme::hWhiteLanesSeparatorHeight, w, PianoRollTheme::hWhiteLanesSeparatorHeight);

        // draw the consecutive white lane
        g.setColour(cWhiteKeysBg);
        g.fillRect(x, y, w, laneHeight);
      }

      ++i;
    }

    // draw bar lines
    i = 0;
    y = 0;
    while (i <= bars) {
      x = i * barWidth;
      g.setColour(cBarSeparatorFg);
      g.fillRect(x, y, PianoRollTheme::vBarSeparatorWidth, h);
      ++i;
      // draw quantize bar lines
      if (quantize > 1) {
        auto sub = barWidth / quantize;
        for (auto j = 1; j < barWidth; j += sub) {
          g.setColour(juce::Colour(cSubBarFg));
          x += sub;
          g.fillRect(x, y, PianoRollTheme::vBarSeparatorWidth, h);
        }
      }
    }
  }

  void resized() override {
  }

  void mouseDown(const juce::MouseEvent &e) override {
    juce::ignoreUnused(e);
    for (auto &n : notes) {
      n->selected = false;
      n->repaint();
    }
  }

  void mouseDoubleClick(const juce::MouseEvent &e) override {
    addNote(e);
  }

  [[nodiscard]] int nearestLane(int y) const {
    auto possibleLaneNumber = y / laneHeight;
    return possibleLaneNumber * laneHeight;
  }

  [[nodiscard]] int nearestBar(int x) const {
    auto possibleBarNumber = x / barWidth;
    return possibleBarNumber * barWidth;
  }

  void addNote(const juce::MouseEvent &e) {
    auto relativeEvent = e.getEventRelativeTo(this);
    auto position = relativeEvent.getPosition();
    auto n = new NoteComponent();
    n->addMouseListener(mouseListener.get(), false);
    n->setBounds(nearestBar(position.x), nearestLane(position.y), barWidth, laneHeight);
    notes.push_back(n);
    addAndMakeVisible(n);
  }

  void removeNote(NoteComponent *note) {
    auto p = std::erase_if(notes, [&](auto &n) { return n == note;});
    if (p != 0) {
      note->removeMouseListener(mouseListener.get());
      removeChildComponent(note);
      delete note;
    }
  }


private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NoteGridComponent)
};
