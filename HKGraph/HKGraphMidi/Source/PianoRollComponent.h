#pragma once

#include "JuceHeader.h"


struct PianoRollTheme {
  static constexpr unsigned int whiteKeysBg = 0xff2a2d31;
  static constexpr unsigned int blackKeysBg = 0xff232528;

  static constexpr unsigned int hWhiteLanesSeparatorFg = 0xff1f2123;
  static constexpr unsigned int hOctaveLanesSeparatorFg = 0xff3a3d42;
  static constexpr unsigned int vBarSeparatorFg = 0xff3a3d42;
  static constexpr unsigned int vSubBarFg = 0xff33363a;

  static constexpr int hLaneSeparatorHeight = 1;
  static constexpr int vBarSeparatorWidth = 1;

  static constexpr unsigned int noteSelectedBg = 0xffe8d5c9;
  static constexpr unsigned int noteUnselectedBg = 0xffb8744a;
  static constexpr float noteBorderWidth = 0.2f;

};

struct NoteComponent : juce::Component {

  bool selected{false};
  bool dragging{false};
  bool resizingRight{false};
  bool resizingLeft{false};
  juce::Rectangle<int> beforeResizingBounds{};
  const juce::Colour cSelectedBg{PianoRollTheme::noteSelectedBg};
  const juce::Colour cUnselectedBg{PianoRollTheme::noteUnselectedBg};
  const juce::Colour cBorderFg{juce::Colours::whitesmoke};
  const float borderWidth{PianoRollTheme::noteBorderWidth};
  juce::Point<int> dragMouseDownPosition{};

  NoteComponent() : juce::Component() {
  }

  ~NoteComponent() override = default;

  void paint(juce::Graphics &g) override {
    auto bounds = getLocalBounds();

    if (selected) {
      g.setColour(cSelectedBg);
    } else {
      g.setColour(cUnselectedBg);
    }
    g.fillRect(bounds);

    juce::Path p;
    p.addRectangle(bounds);
    g.setColour(cBorderFg);
    g.strokePath(p, juce::PathStrokeType(borderWidth));
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
  float scaledWidth{1.0f};
  float scaledHeight{1.0f};

  std::vector<NoteComponent *> notes;

  struct ChildrenMouseListener : juce::MouseListener {

    const juce::MouseCursor rightEdgeResizeCursor{juce::MouseCursor::StandardCursorType::RightEdgeResizeCursor};
    const juce::MouseCursor leftEdgeResizeCursor{juce::MouseCursor::StandardCursorType::LeftEdgeResizeCursor};
    const juce::MouseCursor normalCursor{juce::MouseCursor::StandardCursorType::NormalCursor};

    NoteGridComponent *parent;

    explicit ChildrenMouseListener(NoteGridComponent *p) : juce::MouseListener(), parent(p) {}

    ~ChildrenMouseListener() override = default;

    void mouseMove(const juce::MouseEvent &e) override {
      if (auto note = dynamic_cast<NoteComponent *>(e.originalComponent)) {
        auto relative = e.getEventRelativeTo(note);
        auto localPosition = relative.getPosition();
        auto delta = std::abs(note->getWidth() - localPosition.x);
        if (localPosition.x <= 3) {
          note->setMouseCursor(leftEdgeResizeCursor);
          note->resizingRight = false;
          note->resizingLeft = true;
          note->beforeResizingBounds = note->getBounds();
        } else if (delta <= 3) {
          note->setMouseCursor(rightEdgeResizeCursor);
          note->resizingRight = true;
          note->resizingLeft = false;
          note->beforeResizingBounds = note->getBounds();
        } else {
          note->setMouseCursor(normalCursor);
          note->resizingRight = false;
          note->resizingLeft = false;
        }
      }
    }

    void mouseExit(const juce::MouseEvent &e) override {
      if (auto note = dynamic_cast<NoteComponent *>(e.originalComponent)) {
        note->resizingRight = false;
        note->resizingLeft = false;
        note->setMouseCursor(normalCursor);
      }
    }

    void mouseDown(const juce::MouseEvent &e) override {
      if (auto note = dynamic_cast<NoteComponent *>(e.originalComponent)) {
        note->selected = true;
        note->repaint();
        note->dragMouseDownPosition = e.getEventRelativeTo(note).getMouseDownPosition();
      }
    }

    void mouseDrag(const juce::MouseEvent &e) override {
      if (auto note = dynamic_cast<NoteComponent *>(e.originalComponent)) {
        auto mousePosition = e.getEventRelativeTo(parent).getPosition();
        auto localMousePosition = e.getEventRelativeTo(note).getPosition();
        if (note->resizingRight || note->resizingLeft) {
          if (note->resizingRight && mousePosition.x <= parent->getWidth()) {
            auto delta = localMousePosition - note->dragMouseDownPosition;
            note->setSize(note->beforeResizingBounds.getWidth() + delta.x, note->beforeResizingBounds.getHeight());
          } else if (note->resizingLeft && mousePosition.x >= 0) {
            auto delta = note->dragMouseDownPosition - localMousePosition;
            note->setBounds(
              note->beforeResizingBounds.getX() - delta.x,
              note->beforeResizingBounds.getY(),
              note->beforeResizingBounds.getWidth() + delta.x,
              note->beforeResizingBounds.getHeight());
          }
        } else {
          auto bounds = note->getBounds();
          bounds += localMousePosition - note->dragMouseDownPosition;
          note->dragging = true;
          note->setBounds(bounds);
        }
      }
    }

    void mouseUp(const juce::MouseEvent &e) override {
      if (auto note = dynamic_cast<NoteComponent *>(e.originalComponent)) {
        auto position = e.getEventRelativeTo(parent).getPosition();
        if (note->resizingRight || note->resizingLeft) {
          note->resizingRight = false;
          note->resizingRight = false;
        } else if (note->dragging) {
          note->setBounds(
            parent->nearestBar(position.x, note->getWidth()),
            parent->nearestLane(position.y),
            note->getWidth(),
            note->getHeight()
          );
          note->dragging = false;
        }
      }
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
    int y;
    auto vBarSeparatorWidth = PianoRollTheme::vBarSeparatorWidth / scaledWidth;
    auto hLaneSeparatorHeight = PianoRollTheme::hLaneSeparatorHeight / scaledHeight;
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
        g.fillRect(
          static_cast<float>(x),
          static_cast<float>(y - PianoRollTheme::hLaneSeparatorHeight),
          static_cast<float>(w),
          hLaneSeparatorHeight);

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
      g.fillRect(
        static_cast<float>(x),
        static_cast<float>(y),
        vBarSeparatorWidth,
        static_cast<float>(h));
      ++i;
      // draw quantize bar lines
      if (quantize > 1) {
        auto sub = barWidth / quantize;
        for (auto j = 1; j < barWidth; j += sub) {
          g.setColour(juce::Colour(cSubBarFg));
          x += sub;
          g.fillRect(
            static_cast<float>(x),
            static_cast<float>(y),
            vBarSeparatorWidth,
            static_cast<float>(h));
        }
      }
    }
  }

  void resized() override {
  }

  void mouseDown(const juce::MouseEvent &e) override {
    juce::ignoreUnused(e);
    for (auto &n: notes) {
      n->selected = false;
      n->repaint();
    }
  }

  void mouseDoubleClick(const juce::MouseEvent &e) override {
    addNote(e);
  }

  [[nodiscard]] int nearestLane(int y) const {
    auto possibleLaneNumber = y / laneHeight;
    auto yp = possibleLaneNumber * laneHeight;
    if (yp < 0) yp = 0;
    else if (yp > getHeight() - laneHeight) yp = getHeight() - laneHeight;
    return yp;
  }

  [[nodiscard]] int nearestBar(int x, int width) const {
    auto divisor = barWidth;
    if (quantize > 1) divisor = barWidth / quantize;
    auto possibleBarNumber = x / divisor;
    auto xp = possibleBarNumber * divisor;
    if (xp < 0) xp = 0;
    else if (xp > getWidth() - width) xp = getWidth() - width;
    return xp;
  }

  void addNote(const juce::MouseEvent &e) {
    auto relativeEvent = e.getEventRelativeTo(this);
    auto position = relativeEvent.getPosition();
    auto n = new NoteComponent();
    n->addMouseListener(mouseListener.get(), false);
    n->setBounds(nearestBar(position.x, barWidth), nearestLane(position.y), barWidth, laneHeight);
    notes.push_back(n);
    addAndMakeVisible(n);
  }

  void removeNote(NoteComponent *note) {
    auto p = std::erase_if(notes, [&](auto &n) { return n == note; });
    if (p != 0) {
      note->removeMouseListener(mouseListener.get());
      removeChildComponent(note);
      delete note;
    }
  }


private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NoteGridComponent)
};
