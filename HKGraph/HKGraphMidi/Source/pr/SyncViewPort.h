#pragma once

#include "JuceHeader.h"
#include "NoteGridComponent.h"
#include "TimelineComponent.h"

struct SyncViewport : juce::Viewport {

  std::function<void(SyncViewport *, const juce::Rectangle<int> &)> callback;

  explicit SyncViewport(const juce::String &componentName = juce::String()) :
    juce::Viewport(componentName) {}

  ~SyncViewport() override = default;

  void visibleAreaChanged(const juce::Rectangle<int> &newVisibleArea) override {
    if (callback != nullptr) {
      callback(this, newVisibleArea);
    }
  }

};