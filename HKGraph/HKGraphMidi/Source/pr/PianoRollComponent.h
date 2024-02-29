#pragma once

#include "JuceHeader.h"
#include "SelectionComponent.h"
#include "PianoRollTheme.h"
#include "NoteComponent.h"
#include "NoteGridComponent.h"
#include "TimelineComponent.h"

struct PianoRollComponent : juce::Component {

  TimelineComponent timeline;
  NoteGridComponent noteGrid;

  PianoRollComponent() : juce::Component() {
    addAndMakeVisible(timeline);
    addAndMakeVisible(noteGrid);
  }

  ~PianoRollComponent() override = default;

  void resized() override {
    auto originalBounds = getLocalBounds();
    auto timelineHeight = getLocalPoint(this, juce::Point<int>(0, 32)).y;
    auto origin = getLocalPoint(this, juce::Point<int>());
    timeline.setBounds(
      origin.x,
      origin.y,
      originalBounds.getWidth(),
      static_cast<int>(timelineHeight));
    noteGrid.setBounds(
      origin.x,
      timelineHeight,
      originalBounds.getWidth(),
      originalBounds.getHeight());
  }

  void setScale(float widthFactor, float heightFactor) {
    scaledWidth = widthFactor;
    scaledHeight = heightFactor;
    noteGrid.setScale(widthFactor, heightFactor);
    timeline.setScale(widthFactor, heightFactor);
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
