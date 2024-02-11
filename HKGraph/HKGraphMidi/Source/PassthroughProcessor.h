#pragma once
#include "Processors.h"
#include "NodeProcessor.h"

struct PassthroughProcessor : public NodeProcessor {

  explicit PassthroughProcessor(Graph *graph) :
    NodeProcessor(graph) {
  }

  PassthroughProcessor(Graph *graph, const std::string &name, uint32_t n_ins, uint32_t n_outs)
    : NodeProcessor(graph, name, n_ins, n_outs) {}

  ~PassthroughProcessor() override = default;

  void
  on_data(Graph *graph, const std::optional<const Node::Pin> &pin, Data &data) override {
    juce::ignoreUnused(pin);
    for (auto &[_, p]: m_outs) {
      p.async_dispatch(graph, data);
    }
  }

  [[nodiscard]] std::string typeId() const override {
    return Processors::passthroughProcessor;
  }

  NodeProcessor *clone() override {
    auto c = new PassthroughProcessor(
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
