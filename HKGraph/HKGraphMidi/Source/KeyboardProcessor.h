#pragma once
#include "Processors.h"
#include "NodeProcessor.h"

struct KeyboardProcessor : public NodeProcessor {
  juce::MidiKeyboardState state{};

  explicit KeyboardProcessor(Graph *graph) :
    NodeProcessor(graph) {
  }

  KeyboardProcessor(Graph *graph, const std::string &name, uint32_t n_ins, uint32_t n_outs)
    : NodeProcessor(graph, name, n_ins, n_outs) {}

  ~KeyboardProcessor() override = default;

  void
  on_data(Graph *graph, const std::optional<const Graph::Node::Pin> &pin, Data &data) override {
    juce::ignoreUnused(pin);
    auto input = std::any_cast<juce::MidiBuffer &>(data);
    if (input.getNumEvents() > 0) {
      state.processNextMidiBuffer(input, 0, input.getNumEvents(), true);
      for (auto &[_, p]: m_outs) {
        p.async_dispatch(graph, data);
      }
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
