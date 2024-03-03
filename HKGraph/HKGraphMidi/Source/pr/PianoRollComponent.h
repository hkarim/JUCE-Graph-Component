#pragma once

#include "JuceHeader.h"
#include "SelectionComponent.h"
#include "SyncViewPort.h"
#include "PianoRollTheme.h"
#include "NoteComponent.h"
#include "NoteGridComponent.h"
#include "TimelineComponent.h"
#include "KeysComponent.h"

struct PianoRollComponent : juce::Component {

  static constexpr auto laneHeight = 7;
  static constexpr auto tickWidth = 2;
  static constexpr auto bars = 32;
  static constexpr auto nKeys = 128;
  int quantize{1};

  juce::AudioPlayHead::TimeSignature timeSignature{4, 4};
  TimelineComponent timeline;
  SyncViewport timelineViewPort;
  NoteGridComponent noteGrid;
  SyncViewport noteGridViewPort;
  KeysComponent keyboard;
  SyncViewport keyboardViewPort;

  PianoRollComponent() :
    juce::Component(),
    timeline(timeSignature, bars, tickWidth, quantize),
    noteGrid(timeSignature, laneHeight, nKeys, bars, tickWidth, quantize),
    keyboard(laneHeight, nKeys) {
    keyboardViewPort.setViewedComponent(&keyboard, false);
    keyboardViewPort.setScrollBarsShown(false, false, false, false);
    addAndMakeVisible(keyboardViewPort);

    noteGridViewPort.setViewedComponent(&noteGrid, false);
    addAndMakeVisible(noteGridViewPort);

    timelineViewPort.setViewedComponent(&timeline, false);
    timelineViewPort.setScrollBarsShown(false, false, false, false);
    timeline.onMouseDown = [this](const juce::MouseEvent &e) {
      onTimelineMouseDown(e);
    };
    addAndMakeVisible(timelineViewPort);

    noteGridViewPort.callback = [this](auto *vp, auto &r) -> void {
      return onScroll(vp, r);
    };
  }

  ~PianoRollComponent() override = default;

  void paint(juce::Graphics &g) override {
    g.setColour(juce::Colour(0xff343d48));
    g.fillRect(getLocalBounds());
  }

  void resized() override {
    auto bounds = getLocalBounds();
    auto scrollbarWidth = juce::LookAndFeel::getDefaultLookAndFeel().getDefaultScrollbarWidth();
    auto timelineHeight = 32;
    auto keyboardWidth = 48;

    // keyboard
    keyboard.setSize(keyboardWidth, noteGrid.getHeight() + scrollbarWidth);
    keyboardViewPort.setBounds(
      0,
      timelineHeight,
      keyboard.getWidth(),
      bounds.getHeight() - timelineHeight // compensate for the timeline height
    );

    // timeline
    // to compensate for the visible scrollbar in the grid, since the timeline scrollbars are not visible
    timeline.setSize(noteGrid.getWidth() + scrollbarWidth, timelineHeight);
    timelineViewPort.setBounds(
      keyboardWidth,
      0,
      bounds.getWidth() - keyboardWidth,
      timelineHeight
    );

    // note grid
    noteGridViewPort.setBounds(
      keyboardWidth,
      timelineHeight,
      bounds.getWidth() - keyboardWidth,
      bounds.getHeight() - timelineHeight // compensate for the timeline height
    );

  }

  void setScale(float widthFactor, float heightFactor) {
    scaledWidth = widthFactor;
    scaledHeight = heightFactor;
    noteGrid.setTransform(juce::AffineTransform().scaled(scaledWidth, scaledHeight));

    auto timelineHeight = static_cast<float>(timeline.getHeight()) / scaledHeight;
    auto keyboardWidth = static_cast<float>(keyboard.getWidth()) / scaledWidth;
    noteGrid.setTopLeftPosition(static_cast<int>(keyboardWidth), static_cast<int>(timelineHeight));

    timeline.setTransform(juce::AffineTransform().scaled(scaledWidth, 1.0f));
    keyboard.setTransform(juce::AffineTransform().scaled(1.0f, scaledHeight));
    keyboard.setScale(widthFactor, heightFactor);
    noteGrid.setScale(widthFactor, heightFactor);
    timeline.setScale(widthFactor, heightFactor);
  }

  void onScroll(SyncViewport *vp, const juce::Rectangle<int> &) {
    auto horizontalRange = vp->getHorizontalScrollBar().getCurrentRangeStart();
    timelineViewPort.getHorizontalScrollBar().setCurrentRangeStart(horizontalRange);
    auto verticalRange = vp->getVerticalScrollBar().getCurrentRangeStart();
    keyboardViewPort.getVerticalScrollBar().setCurrentRangeStart(verticalRange);
  }

  void onTimelineMouseDown(const juce::MouseEvent &e) {
    auto x = e.getPosition().toFloat().x;
    timeline.playHeadPosition = x;
    noteGrid.playHeadPosition = x;
    repaint();
  }

  [[nodiscard]] int getQuantize() const {
    return quantize;
  }

  void setQuantize(int v) {
    quantize = v;
    noteGrid.quantize = v;
    timeline.quantize = v;
    repaint();
  }

private:
  float scaledWidth{1.0f};
  float scaledHeight{1.0f};


  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PianoRollComponent)
};
