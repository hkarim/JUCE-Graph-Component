#pragma once

#include "JuceHeader.h"
#include "SelectionComponent.h"
#include "SyncViewPort.h"
#include "PianoRollTheme.h"
#include "NoteComponent.h"
#include "NoteGridComponent.h"
#include "TimelineComponent.h"

struct PianoRollComponent : juce::Component {

  TimelineComponent timeline;
  NoteGridComponent noteGrid;
  SyncViewport timelineViewPort;
  SyncViewport noteGridViewPort;

  PianoRollComponent() : juce::Component() {
    addAndMakeVisible(timeline);
    noteGridViewPort.setViewedComponent(&noteGrid, false);
    addAndMakeVisible(noteGridViewPort);
    timelineViewPort.setViewedComponent(&timeline, false);
    timelineViewPort.setScrollBarsShown(false, false, false, false);
    addAndMakeVisible(timelineViewPort);

    noteGridViewPort.callback = [this](auto *vp, auto &r) -> void {
      return onScroll(vp, r);
    };
  }

  ~PianoRollComponent() override = default;

  void resized() override {
    auto originalBounds = getLocalBounds();
    auto timelineHeight = 32;
    auto w = bars * barWidth;
    // to compensate for the visible scrollbar in the grid, since the timeline scrollbars are not visible
    auto vScrollbarWidth = juce::LookAndFeel::getDefaultLookAndFeel().getDefaultScrollbarWidth();
    timeline.setSize(w + vScrollbarWidth, timelineHeight);
    timelineViewPort.setBounds(
      0,
      0,
      originalBounds.getWidth(),
      timelineHeight
    );
    noteGrid.setSize(w, 127 * laneHeight);
    noteGridViewPort.setBounds(
      0,
      timelineHeight,
      originalBounds.getWidth(),
      originalBounds.getHeight() - timelineHeight // compensate for the scrollbar height
    );
  }

  void setScale(float widthFactor, float heightFactor) {
    scaledWidth = widthFactor;
    scaledHeight = heightFactor;
    noteGrid.setTransform(juce::AffineTransform().scaled(scaledWidth, scaledHeight));
    auto tlh = static_cast<float>(timeline.getHeight()) / scaledHeight;
    noteGrid.setTopLeftPosition(0, static_cast<int>(tlh));
    timeline.setTransform(juce::AffineTransform().scaled(scaledWidth, 1.0f));
    noteGrid.setScale(widthFactor, heightFactor);
    timeline.setScale(widthFactor, heightFactor);
  }

  void onScroll(SyncViewport *vp, const juce::Rectangle<int> &) {
    auto range = vp->getHorizontalScrollBar().getCurrentRangeStart();
    timelineViewPort.getHorizontalScrollBar().setCurrentRangeStart(range);
  }

  [[nodiscard]] int getBarWidth() const {
    return barWidth;
  }

  void setBarWidth(int v) {
    barWidth = v;
    noteGrid.barWidth = v;
  }

  [[nodiscard]] int getLaneHeight() const {
    return laneHeight;
  }

  void setLaneHeight(int v) {
    laneHeight = v;
    noteGrid.laneHeight = v;
  }

  [[nodiscard]] int getBars() const {
    return bars;
  }

  void setBars(int v) {
    bars = v;
    noteGrid.bars = v;
  }

  [[nodiscard]] int getQuantize() const {
    return quantize;
  }

  void setQuantize(int v) {
    quantize = v;
    noteGrid.quantize = v;
  }

private:
  float scaledWidth{1.0f};
  float scaledHeight{1.0f};
  int barWidth{64};
  int laneHeight{8};
  int bars{32};
  int quantize{1};
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PianoRollComponent)
};
