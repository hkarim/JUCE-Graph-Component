#pragma once

#include "Processors.h"
#include "NodeProcessor.h"
#include "PlaybackProcessor.h"

struct KeyboardProcessor : public PlaybackProcessor {
  juce::MidiKeyboardState keyboardState{};
  juce::MidiMessageCollector keyboardMessageCollector;
  bool hasCalledReset{false};

  explicit KeyboardProcessor(Graph *graph) :
    PlaybackProcessor(graph) {
    keyboardMessageCollector.reset(512);
    hasCalledReset = true;
    keyboardState.addListener(&keyboardMessageCollector);
  }

  KeyboardProcessor(Graph *graph, const std::string &name, uint32_t n_ins, uint32_t n_outs)
    : PlaybackProcessor(graph, name, n_ins, n_outs) {
    keyboardMessageCollector.reset(512);
    hasCalledReset = true;
    keyboardState.addListener(&keyboardMessageCollector);
  }

  ~KeyboardProcessor() override {
    keyboardState.removeListener(&keyboardMessageCollector);
  }

  void prepareToPlay(double playbackSampleRate, int playbackSamplesPerBlock) override {
    PlaybackProcessor::prepareToPlay(playbackSampleRate, playbackSamplesPerBlock);
    keyboardMessageCollector.reset(playbackSampleRate);
    hasCalledReset = true;
  }

  void
  on_data(Graph *graph, const std::optional<const Graph::Node::Pin> &pin, Data &data) override {
    juce::ignoreUnused(pin);
    auto input = std::any_cast<Block>(data);
    juce::MidiBuffer output;
    if (!input.midiBuffer.isEmpty()) {
      output.addEvents(input.midiBuffer, 0, -1, 0);
      keyboardState.processNextMidiBuffer(output, 0, output.getNumEvents(), true);
    }
    if (hasCalledReset) {
      keyboardMessageCollector.removeNextBlockOfMessages(output, input.audioBuffer.getNumSamples());
    }
    Data result = std::make_any<Block>(input.audioBuffer, output);
    for (auto &[_, p]: m_outs) {
      p.async_dispatch(graph, result);
    }
  }

  [[nodiscard]] std::string typeId() const override {
    return Processors::keyboardProcessor;
  }

  NodeProcessor *clone() override {
    auto c = new KeyboardProcessor(
      m_graph,
      m_name,
      static_cast<std::uint32_t>(m_ins.size()),
      static_cast<std::uint32_t>(m_outs.size()));
    c->m_muted = m_muted;
    return c;
  }

  juce::Component *createEditor(const GraphViewTheme &theme) override;
};
