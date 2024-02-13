#pragma once
#include "Processors.h"
#include "NodeProcessor.h"

struct MidiOutNodeProcessor : public NodeProcessor {
  juce::MidiBuffer output;

  explicit MidiOutNodeProcessor(Graph *graph) :
    NodeProcessor(graph) {
  }

  MidiOutNodeProcessor(Graph *graph, const std::string &name, uint32_t n_ins, uint32_t n_outs)
    : NodeProcessor(graph, name, n_ins, n_outs) {}

  ~MidiOutNodeProcessor() override = default;

  void
  on_data(Graph *graph, const std::optional<const Node::Pin> &pin, Data &data) override {
    juce::ignoreUnused(graph, pin);
    auto input = std::any_cast<Block>(data);
    output.addEvents(input.midiBuffer, 0, -1, 0);
  }

  [[nodiscard]] std::string typeId() const override {
    return Processors::midiOutNodeProcessor;
  }

  NodeProcessor *clone() override {
    auto c = new MidiInNodeProcessor(
      m_graph,
      m_name,
      static_cast<uint32_t>(m_ins.size()),
      static_cast<uint32_t>(m_outs.size()));
    c->set_muted(m_graph, is_muted());
    return c;
  }

  juce::Component *createEditor(const GraphViewTheme &theme) override {
    juce::ignoreUnused(theme);
    return nullptr;
  }
};