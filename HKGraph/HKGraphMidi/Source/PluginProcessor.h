#pragma once

#include "JuceHeader.h"
#include "Graph.h"
#include "MidiInNodeProcessor.h"
#include "MidiOutNodeProcessor.h"
#include "NodeDescriptor.h"
#include "ProcessorRegistry.h"

struct Preferences {
  int editorWidth = 800;
  int editorHeight = 400;
};

class AudioPluginAudioProcessor final : public juce::AudioProcessor, public Graph::Listener {

public:
  Graph *graph;
  std::unordered_map<uuid, std::unique_ptr<NodeDescriptor>> nodeDescriptors;
  MidiInNodeProcessor *midiIn;
  MidiOutNodeProcessor *midiOut;
  bool m_dirty{false};
  Preferences preferences;
  juce::ValueTree parameters;

  AudioPluginAudioProcessor();

  ~AudioPluginAudioProcessor() override;

  void on_graph_event(const Graph::Event &event) override;

  void prepareToPlay(double sampleRate, int samplesPerBlock) override;

  void releaseResources() override;

  bool isBusesLayoutSupported(const BusesLayout &layouts) const override;

  void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

  using AudioProcessor::processBlock;

  juce::AudioProcessorEditor *createEditor() override;

  bool hasEditor() const override;

  const juce::String getName() const override;

  bool acceptsMidi() const override;

  bool producesMidi() const override;

  bool isMidiEffect() const override;

  double getTailLengthSeconds() const override;

  int getNumPrograms() override;

  int getCurrentProgram() override;

  void setCurrentProgram(int index) override;

  const juce::String getProgramName(int index) override;

  void changeProgramName(int index, const juce::String &newName) override;

  void saveState();

  void restoreState();

  void recoverMidiInOut();

  void assignMidiInOutDescriptors();

  void getStateInformation(juce::MemoryBlock &destData) override;

  void setStateInformation(const void *data, int sizeInBytes) override;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessor)
};
