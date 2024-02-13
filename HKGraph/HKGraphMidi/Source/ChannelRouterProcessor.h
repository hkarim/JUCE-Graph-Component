#pragma once
#include "NodeProcessor.h"
#include "RangeParameter.h"
#include "Processors.h"

struct ChannelRouterProcessor : public NodeProcessor {
  IntRangeParameter m_parameter;

  explicit ChannelRouterProcessor(Graph *graph) :
    NodeProcessor(graph),
    m_parameter(1, 16, 1, 1) {

  }

  ChannelRouterProcessor(Graph *graph, const std::string &name, uint32_t n_ins, uint32_t n_outs)
    : NodeProcessor(graph, name, n_ins, n_outs),
      m_parameter(1, 16, 1, 1) {
  }

  ~ChannelRouterProcessor() override = default;

  void
  on_data(Graph *graph, const std::optional<const Graph::Node::Pin> &pin, Data &data) override {
    juce::ignoreUnused(pin);
    auto shift = m_parameter.value;
    auto input = std::any_cast<Block>(data);
    juce::MidiBuffer output;
    /////
    if (m_parameter.changed) {
      for (auto i = 1; i <= 16; i++) {
        output.addEvent(juce::MidiMessage::allNotesOff(i), 0);
      }
      m_parameter.changed = false;
    }
    /////
    for (auto m: input.midiBuffer) {
      auto message = m.getMessage();
      message.setChannel(shift);
      output.addEvent(message, m.samplePosition);
    }
    Data outputData = std::make_any<Block>(input.audioBuffer, output);
    for (auto &[_, p]: m_outs) {
      p.async_dispatch(graph, outputData);
    }
  }

  [[nodiscard]] std::string typeId() const override {
    return Processors::channelRouterProcessor;
  }

  void saveState(juce::ValueTree& nodeTree) override {
    nodeTree.setProperty("ChannelRouterProcessor::parameter", juce::var(m_parameter.value), nullptr);
  }

  void restoreState(const juce::ValueTree& nodeTree) override {
    m_parameter.value = nodeTree.getProperty("ChannelRouterProcessor::parameter");
  }

  NodeProcessor *clone() override {
    auto c =
      new ChannelRouterProcessor(
        m_graph,
        m_name,
        static_cast<std::uint32_t>(m_ins.size()),
        static_cast<std::uint32_t>(m_outs.size()));
    c->m_muted = m_muted;
    c->m_parameter.value = m_parameter.value;
    return c;
  }

  juce::Component *createEditor(const GraphViewTheme &theme) override;

};
