#pragma once

#include "Graph.h"
#include "Processors.h"

struct ChannelSplitterProcessor : public NodeProcessor {
  std::unordered_map<int, uuid> m_ordered_pins;

  explicit ChannelSplitterProcessor(Graph *graph) :
    NodeProcessor(graph) {
  }

  ChannelSplitterProcessor(Graph *graph, const std::string &name, uint32_t n_ins, uint32_t n_outs)
    : NodeProcessor(graph, name, n_ins, n_outs) {
    for (auto &[id, p]: m_outs) {
      m_ordered_pins[static_cast<int>(p.m_order)] = id;
    }
  }

  ~ChannelSplitterProcessor() override = default;

  void
  on_data(Graph *graph, const std::optional<const Node::Pin> &pin, Data &data) override {
    juce::ignoreUnused(pin);
    auto input = std::any_cast<Block>(data);
    std::unordered_map<int, juce::MidiBuffer> output;
    for (auto m: input.midiBuffer) {
      output[m.getMessage().getChannel() - 1].addEvent(m.getMessage(), m.samplePosition);
    }
    for (auto &[index, channelOutput]: output) {
      auto uuid_ptr = m_ordered_pins.find(index);
      if (uuid_ptr != std::end(m_ordered_pins)) {
        auto pin_ptr = m_outs.find(uuid_ptr->second);
        if (pin_ptr != std::end(m_outs)) {
          Data channelData = std::make_any<Block>(input.audioBuffer, channelOutput);
          pin_ptr->second.async_dispatch(graph, channelData);
        }
      }
    }
  }

  [[nodiscard]] std::string typeId() const override {
    return Processors::channelSplitterProcessor;
  }

  void saveState(juce::ValueTree& nodeTree) override {
    juce::ignoreUnused(nodeTree);
  }

  void restoreState(const juce::ValueTree& nodeTree) override {
    juce::ignoreUnused(nodeTree);
    for (auto &[id, p]: m_outs) {
      m_ordered_pins[static_cast<int>(p.m_order)] = id;
    }
  }

  NodeProcessor *clone() override {
    auto c = new ChannelSplitterProcessor(
      m_graph,
      m_name,
      static_cast<uint32_t>(m_ins.size()),
      static_cast<uint32_t>(m_outs.size()));
    c->m_muted = m_muted;
    return c;
  }

  juce::Component *createEditor(const GraphViewTheme &theme) override {
    juce::ignoreUnused(theme);
    return nullptr;
  }
};
