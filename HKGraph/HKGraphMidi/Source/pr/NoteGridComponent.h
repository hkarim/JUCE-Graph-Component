#pragma once

#include "JuceHeader.h"
#include "GraphViewTheme.h"
#include "SelectionComponent.h"
#include "PianoRollTheme.h"
#include "NoteComponent.h"

struct NoteGridComponent : juce::Component {

  const juce::Colour cWhiteKeysBg{PianoRollTheme::whiteKeysBg};
  const juce::Colour cBlackKeysBg{PianoRollTheme::blackKeysBg};
  const juce::Colour cWhiteLanesLineFg{PianoRollTheme::hWhiteLanesSeparatorFg};
  const juce::Colour cOctaveLanesSeparatorFg{PianoRollTheme::hOctaveLanesSeparatorFg};
  const juce::Colour cBarSeparatorFg{PianoRollTheme::vBarSeparatorFg};
  const juce::Colour cSubBarFg{PianoRollTheme::vSubBarFg};

  juce::AudioPlayHead::TimeSignature timeSignature{};
  int laneHeight{8};
  int nKeys{128};
  int bars{32};
  int barWidth{64};
  int quantize{1};
  bool freeResize{false};


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
        parent->updateNoteModel(note);
      }
    }

    void mouseExit(const juce::MouseEvent &e) override {
      if (auto note = dynamic_cast<NoteComponent *>(e.originalComponent)) {
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
          for (auto &n: parent->notes) {
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

  struct NoteConstrainer : juce::ComponentBoundsConstrainer {
    NoteGridComponent *parent;

    explicit NoteConstrainer(NoteGridComponent *p) : juce::ComponentBoundsConstrainer(), parent(p) {}

    ~NoteConstrainer() override = default;

    void checkBounds(juce::Rectangle<int> &bounds,
                     const juce::Rectangle<int> &previousBounds,
                     const juce::Rectangle<int> &,
                     bool,
                     bool isStretchingLeft,
                     bool,
                     bool isStretchingRight) override {
      auto xp = previousBounds.getX();
      auto yp = previousBounds.getY();
      auto wp = previousBounds.getWidth();
      auto hp = previousBounds.getHeight();

      auto x = bounds.getX();
      auto w = bounds.getWidth();

      // boundaries
      if (x < 0) {
        x = 0;
        bounds.setBounds(x, yp, wp, hp);
        return;
      }

      if (x + w > parent->getWidth()) {
        bounds.setBounds(x, yp, parent->getWidth() - x, hp);
        return;
      }

      if (parent->freeResize) {
        if (w < 16) {
          bounds.setBounds(xp, yp, wp, hp);
        }
      } else {
        // snap
        if (isStretchingLeft) {
          auto xs = parent->nearestBar(x, w);
          auto xr = x + w;
          if (xr > xs) {
            bounds.setLeft(xs);
          } else {
            //bounds.setBounds(x, yp, wp, hp);
            bounds.setBounds(xp, yp, wp, hp);
          }
        } else if (isStretchingRight) {
          auto xr = x + w;
          auto xs = parent->nearestBar(xr, 0);
          w = xs - x;
          auto min = parent->barWidth / (parent->quantize * parent->timeSignature.numerator);
          if (w >= min) {
            bounds.setBounds(x, yp, w, hp);
          } else {
            bounds.setBounds(xp, yp, wp, hp);
          }
        }
      }

    }
  };

  std::unique_ptr<NoteConstrainer> noteConstrainer;

  NoteGridComponent(juce::AudioPlayHead::TimeSignature ts, int gridLaneHeight, int numberOfKeys, int numberOfBars,
                    int gridBarWidth) :
    juce::Component(),
    timeSignature(ts),
    laneHeight(gridLaneHeight),
    nKeys(numberOfKeys),
    bars(numberOfBars),
    barWidth(gridBarWidth),
    mouseListener(new ChildrenMouseListener(this)),
    noteConstrainer(new NoteConstrainer(this)) {
    setSize(bars * barWidth, numberOfKeys * laneHeight);
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
    auto zeroBasedKeys = nKeys - 1;
    while (i <= zeroBasedKeys) {
      auto n = zeroBasedKeys - i; // the note number
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
          //static_cast<float>(y - PianoRollTheme::hLaneSeparatorHeight),
          static_cast<float>(static_cast<float>(y) - hLaneSeparatorHeight),
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
        auto sub = barWidth / (quantize * timeSignature.numerator);
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


  void noteMouseDown(NoteComponent *note, const juce::MouseEvent &e) {
    if (noteMultiSelectionOn && !note->selected) {
      noteMultiSelectionOn = false;
    }
    if (noteMultiSelectionOn && note->selected) {
      for (auto &n: notes) {
        n->dragMouseDownPosition = e.getEventRelativeTo(n).getMouseDownPosition();
      }
    } else {
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
      for (auto &n: notes) {
        if (n->selected) startNoteDrag(n, e);
      }
    } else {
      startNoteDrag(note, e);
    }
  }

  void noteMouseUp(NoteComponent *note, const juce::MouseEvent &e) {
    if (note->dragging) {
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

  void startNoteDrag(NoteComponent *note, const juce::MouseEvent &e) const {
    auto localMousePosition = e.getEventRelativeTo(note).getPosition();
    auto bounds = note->getBounds();
    bounds += localMousePosition - note->dragMouseDownPosition;
    note->dragging = true;
    updateNoteModel(note);
    note->setBounds(bounds);
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

  [[nodiscard]] int nearestLane(int y) const {
    auto possibleLaneNumber = y / laneHeight;
    auto yp = possibleLaneNumber * laneHeight;
    if (yp < 0) yp = 0;
    else if (yp > getHeight() - laneHeight) yp = getHeight() - laneHeight;
    return yp;
  }

  [[nodiscard]] int nearestBar(int x, int width) const {
    auto divisor = barWidth;
    if (quantize > 1) divisor = barWidth / (quantize * timeSignature.numerator);
    auto possibleBarNumber = x / divisor;
    auto xp = possibleBarNumber * divisor;
    if (xp < 0) xp = 0;
    else if (xp > getWidth() - width) xp = getWidth() - width;
    return xp;
  }

  void addNote(const juce::MouseEvent &e) {
    auto relativeEvent = e.getEventRelativeTo(this);
    auto position = relativeEvent.getPosition();
    auto n = new NoteComponent(noteConstrainer.get());
    n->addMouseListener(mouseListener.get(), false);
    n->setBounds(nearestBar(position.x, barWidth), nearestLane(position.y), barWidth, laneHeight);
    updateNoteModel(n);
    n->callback = [this](NoteComponent *, bool freeResizeOn) {
      this->freeResize = freeResizeOn;
    };
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
