#pragma once

#include "JuceHeader.h"
#include "Graph.h"
#include "GraphViewTheme.h"

struct NodeProcessor : public Graph::Node {
  Graph *m_graph;

  explicit NodeProcessor(Graph *graph) : m_graph(graph) {
  }

  NodeProcessor(
    Graph *graph,
    const std::string &name,
    std::uint32_t n_ins,
    std::uint32_t n_outs)
    : m_graph(graph) {
    m_id = graph->make_uuid();
    m_name = name;
    for (auto i = 0u; i < n_ins; ++i) {
      auto id = graph->make_uuid();
      m_ins[id] = Graph::Node::Pin{
        .m_id = id,
        .m_order = static_cast<std::uint32_t>(i),
        .m_kind = Graph::Node::PinKind::In,
        .m_owner_node_id = m_id,
      };
    }

    for (auto i = 0u; i < n_outs; ++i) {
      auto id = graph->make_uuid();
      m_outs[id] = Graph::Node::Pin{
        .m_id = id,
        .m_order = static_cast<std::uint32_t>(i),
        .m_kind = Graph::Node::PinKind::Out,
        .m_owner_node_id = m_id,
      };
    }
  }

  ~NodeProcessor() override = default;

  [[nodiscard]] virtual std::string typeId() const = 0;

  virtual void saveState(juce::ValueTree &nodeTree) {
    juce::ignoreUnused(nodeTree);
  }

  virtual void restoreState(const juce::ValueTree &nodeTree) {
    juce::ignoreUnused(nodeTree);
  }

  virtual NodeProcessor *clone() = 0;

  virtual juce::Component *createEditor(const GraphViewTheme &theme) = 0;
};
