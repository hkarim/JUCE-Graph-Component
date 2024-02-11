#include "PluginProcessor.h"
#include "PluginEditor.h"

AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor &p)
  : AudioProcessorEditor(&p),
    processorRef(p),
    graphEditor(std::make_unique<GraphEditor>(p.graph)) {
  ///
  graphEditor->restoreUI(p.nodeDescriptors);
  ///
  graphEditor->setSize(p.preferences.editorWidth, p.preferences.editorHeight);

  addAndMakeVisible(graphEditor.get());

  setSize(p.preferences.editorWidth, p.preferences.editorHeight);
  setResizable(true, true);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor() {
  graphEditor->recordUI(processorRef.nodeDescriptors);
}

void AudioPluginAudioProcessorEditor::paint(juce::Graphics &g) {
  // (Our component is opaque, so we must completely fill the background with a solid colour)
  g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
  g.setColour(juce::Colours::white);
  g.setFont(15.0f);
}

void AudioPluginAudioProcessorEditor::resized() {
  graphEditor->setBounds(getLocalBounds());
  processorRef.preferences.editorWidth = graphEditor->getWidth();
  processorRef.preferences.editorHeight = graphEditor->getHeight();
}
