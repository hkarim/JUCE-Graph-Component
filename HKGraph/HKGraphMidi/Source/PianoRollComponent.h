#pragma once

#include "JuceHeader.h"
#include "SelectionComponent.h"

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

struct NoteModel {
  int lane{0};
  int start{0};
  int end{0};
  int velocity{0};
  float scaledWidth{1.0f};
  float scaledHeight{1.0f};
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
  NoteModel model{};

  NoteComponent() : juce::Component() {
  }

  ~NoteComponent() override = default;

  void paint(juce::Graphics &g) override {
    auto bounds = getLocalBounds();

    // body
    if (selected) {
      g.setColour(cSelectedBg);
    } else {
      g.setColour(cUnselectedBg);
    }
    g.fillRect(bounds);

    // border
    juce::Path p;
    p.addRectangle(bounds);
    g.setColour(cBorderFg);
    g.strokePath(p, juce::PathStrokeType(borderWidth));

    // text
    auto h = static_cast<float>(bounds.getHeight()) * model.scaledHeight;
    auto f = g.getCurrentFont();
    auto fh = f.getHeight();
    if (h >= fh) {
      auto n = 127 - model.lane;
      auto at = juce::AffineTransform().scaled(1.0f / model.scaledWidth, 1.0f / model.scaledHeight);
      auto delta = std::abs(h - fh);
      auto spacing = delta / 2;
      if (selected) {
        g.setColour(cUnselectedBg);
      } else {
        g.setColour(cSelectedBg);
      }
      g.addTransform(at);
      g.drawText(
        juce::MidiMessage::getMidiNoteName(n, true, true, 3),
        3,
        static_cast<int>(spacing),
        bounds.getWidth(),
        bounds.getHeight(),
        juce::Justification::topLeft,
        false);
    }

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


  const juce::MouseCursor rightEdgeResizeCursor{juce::MouseCursor::StandardCursorType::RightEdgeResizeCursor};
  const juce::MouseCursor leftEdgeResizeCursor{juce::MouseCursor::StandardCursorType::LeftEdgeResizeCursor};
  const juce::MouseCursor normalCursor{juce::MouseCursor::StandardCursorType::NormalCursor};
  std::vector<NoteComponent *> notes;
  bool noteMultiSelectionOn{false};
  GraphViewTheme theme;
  SelectionComponent selector{theme.cSelectionBackground};

  struct ChildrenMouseListener : juce::MouseListener {
    NoteGridComponent *parent;

    explicit ChildrenMouseListener(NoteGridComponent *p) : juce::MouseListener(), parent(p) {}

    ~ChildrenMouseListener() override = default;

    void mouseMove(const juce::MouseEvent &e) override {
      if (auto note = dynamic_cast<NoteComponent *>(e.originalComponent)) {
        parent->noteMouseMove(note, e);
        parent->updateNoteModel(note);
      }
    }

    void mouseExit(const juce::MouseEvent &e) override {
      if (auto note = dynamic_cast<NoteComponent *>(e.originalComponent)) {
        parent->noteMouseExit(note, e);
        parent->updateNoteModel(note);
      }
    }

    void mouseDown(const juce::MouseEvent &e) override {
      if (auto note = dynamic_cast<NoteComponent *>(e.originalComponent)) {
        parent->noteMouseDown(note, e);
        parent->updateNoteModel(note);
      }
    }

    void mouseDrag(const juce::MouseEvent &e) override {
      if (auto note = dynamic_cast<NoteComponent *>(e.originalComponent)) {
        parent->noteMouseDrag(note, e);
      }
    }

    void mouseUp(const juce::MouseEvent &e) override {
      if (auto note = dynamic_cast<NoteComponent *>(e.originalComponent)) {
        parent->noteMouseUp(note, e);
        if (parent->noteMultiSelectionOn) {
          for (auto &n : parent->notes) {
            parent->updateNoteModel(n);
            n->repaint();
          }
        } else {
          parent->updateNoteModel(note);
        }
      }
    }

    void mouseDoubleClick(const juce::MouseEvent &e) override {
      if (auto note = dynamic_cast<NoteComponent *>(e.originalComponent)) {
        parent->noteMouseDoubleClick(note, e);
        parent->updateNoteModel(note);
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
    for (auto &n: notes) {
      n->selected = false;
      n->repaint();
    }
    noteMultiSelectionOn = false;
    // selector
    auto position = e.getMouseDownPosition();
    selector.setBounds(position.x, position.y, 10, 10);
    addAndMakeVisible(selector);
  }

  void mouseDoubleClick(const juce::MouseEvent &e) override {
    addNote(e);
  }

  void mouseDrag(const juce::MouseEvent &e) override {
    auto position = e.getMouseDownPosition();
    auto offset = e.getOffsetFromDragStart();
    selector.calculateBounds(position, offset);
    // handle node selection
    auto numSelectedNotes = 0;
    for (auto &n: notes) {
      auto selected = selector.getBounds().intersects(n->getBounds());
      n->selected = selected;
      n->repaint();
      if (selected) ++numSelectedNotes;
    }
    noteMultiSelectionOn = numSelectedNotes > 0;
  }

  void mouseUp(const juce::MouseEvent &) override {
    removeChildComponent(&selector);
  }

  void noteMouseMove(NoteComponent *note, const juce::MouseEvent &e) {
    if (!noteMultiSelectionOn) {
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

  void noteMouseExit(NoteComponent *note, const juce::MouseEvent &) {
    note->resizingRight = false;
    note->resizingLeft = false;
    note->setMouseCursor(normalCursor);
  }

  void noteMouseDown(NoteComponent *note, const juce::MouseEvent &e) {
    if (noteMultiSelectionOn && !note->selected) {
      noteMultiSelectionOn = false;
    }
    if (noteMultiSelectionOn && note->selected) {
      for (auto &n: notes) {
        n->dragMouseDownPosition = e.getEventRelativeTo(n).getMouseDownPosition();
      }
    }
    else {
      note->selected = true;
      note->repaint();
      for (auto &n: notes) {
        n->dragMouseDownPosition = e.getEventRelativeTo(n).getMouseDownPosition();
        if (n != note && n->selected) {
          n->selected = false;
          n->repaint();
        }
      }
    }
  }

  void noteMouseDrag(NoteComponent *note, const juce::MouseEvent &e) {
    if (noteMultiSelectionOn) {
      for (auto &n : notes) {
        if (n->selected) startNoteDrag(n, e);
      }
    }
    else {
      startNoteDrag(note, e) ;
    }
  }

  void noteMouseUp(NoteComponent *note, const juce::MouseEvent &e) {
    if (note->resizingRight || note->resizingLeft) {
      note->resizingRight = false;
      note->resizingRight = false;
    } else if (note->dragging) {
       if (noteMultiSelectionOn) {
         for (auto &n: notes) {
           if (n->selected) endNoteDrag(n, e);
         }
       } else {
         endNoteDrag(note, e);
       }
    }
  }

  void noteMouseDoubleClick(NoteComponent *note, const juce::MouseEvent &) {
    removeNote(note);
  }

  void startNoteDrag(NoteComponent * note, const juce::MouseEvent &e) {
    auto localMousePosition = e.getEventRelativeTo(note).getPosition();
    if ((note->resizingRight || note->resizingLeft) && !noteMultiSelectionOn) { // handle resizing first
      resizeNote(note, e);
    } else { // handle dragging the whole note
      auto bounds = note->getBounds();
      bounds += localMousePosition - note->dragMouseDownPosition;
      note->dragging = true;
      updateNoteModel(note);
      note->setBounds(bounds);
    }
  }

  void endNoteDrag(NoteComponent *note, const juce::MouseEvent &e) const {
    //auto mousePosition = e.getEventRelativeTo(this).getPosition();
    auto notePosition = note->getPosition();
    auto x = nearestBar(notePosition.x, note->getWidth());
    if (e.mods.isShiftDown()) {
      x = notePosition.x;
    }
    note->setBounds(
      x,
      //nearestLane(mousePosition.y),
      nearestLane(notePosition.y),
      note->getWidth(),
      note->getHeight()
    );
    note->dragging = false;
  }

  void resizeNote(NoteComponent *note, const juce::MouseEvent &e) {
    auto mousePosition = e.getEventRelativeTo(this).getPosition();
    auto localMousePosition = e.getEventRelativeTo(note).getPosition();
    if (note->resizingRight && mousePosition.x <= this->getWidth()) {
      auto delta = localMousePosition - note->dragMouseDownPosition;
      if (e.mods.isShiftDown()) { // freely resize from the right if shift is down
        note->setSize(
          note->beforeResizingBounds.getWidth() + delta.x,
          note->beforeResizingBounds.getHeight());
      } else { // snap to the nearest bar if shift is **not** down
        auto w = nearestBar(note->beforeResizingBounds.getWidth() + delta.x, note->getWidth());
        auto x = nearestBar(note->getPosition().x, note->beforeResizingBounds.getWidth());
        auto compensation = x - note->getPosition().x;
        w = w + compensation; // if the left position is not aligned with a bar, we need to compensate
        if (w == 0) w = note->beforeResizingBounds.getWidth();
        note->setSize(
          w,
          note->beforeResizingBounds.getHeight());
      }
    } else if (note->resizingLeft && mousePosition.x >= 0) {
      auto delta = note->dragMouseDownPosition - localMousePosition;
      if (e.mods.isShiftDown()) { // freely resize from the left if shift is down
        note->setBounds(
          note->beforeResizingBounds.getX() - delta.x,
          note->beforeResizingBounds.getY(),
          note->beforeResizingBounds.getWidth() + delta.x,
          note->beforeResizingBounds.getHeight());
      } else { // snap to the nearest bar if shift is **not** down
        auto x = note->beforeResizingBounds.getX();
        auto y = note->beforeResizingBounds.getY();
        auto w = note->beforeResizingBounds.getWidth();
        auto h = note->beforeResizingBounds.getHeight();
        auto shift = x - delta.x;
        auto xp = nearestBar(shift, note->beforeResizingBounds.getWidth());
        auto compensation = x - xp;
        note->setBounds(xp, y, w + compensation, h);
      }
    }
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
    updateNoteModel(n);
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

  void updateNoteModel(NoteComponent *note) const {
    note->model.start = note->getX();
    note->model.end = note->getWidth();
    note->model.lane = note->getY() / laneHeight;
    note->model.scaledWidth = scaledWidth;
    note->model.scaledHeight = scaledHeight;
  }

  void setScale(float widthFactor, float heightFactor) {
    scaledWidth = widthFactor;
    scaledHeight = heightFactor;
    for (auto &n: notes) {
      n->model.scaledWidth = scaledWidth;
      n->model.scaledHeight = scaledHeight;
      n->repaint();
    }
  }

private:
  float scaledWidth{1.0f};
  float scaledHeight{1.0f};
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NoteGridComponent)
};
