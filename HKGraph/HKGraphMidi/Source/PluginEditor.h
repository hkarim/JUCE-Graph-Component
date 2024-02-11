#pragma once

#include "PluginProcessor.h"
#include "GraphEditor.h"

class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor {
public:
  explicit AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor &);

  ~AudioPluginAudioProcessorEditor() override;

  void paint(juce::Graphics &) override;

  void resized() override;

private:

  AudioPluginAudioProcessor &processorRef;
  juce::TooltipWindow tooltipWindow;
  std::unique_ptr<GraphEditor> graphEditor;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};
