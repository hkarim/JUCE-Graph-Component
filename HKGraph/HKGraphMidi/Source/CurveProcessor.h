#pragma once

#include "NodeProcessor.h"
#include "CurveEditor.h"
#include "Processors.h"

struct CurveProcessor : public NodeProcessor {

  ce::CurveEditorModel<float> model;

  CurveProcessor(Graph *graph, const std::string &name, uint32_t n_ins, uint32_t n_outs,
                 ce::CurveEditorModel<float> &&curveEditorModel)
    : NodeProcessor(graph, name, n_ins, n_outs),
      model(std::move(curveEditorModel)) {}

  explicit CurveProcessor(Graph *graph, ce::CurveEditorModel<float> &&curveEditorModel) :
    NodeProcessor(graph),
    model(std::move(curveEditorModel)) {}

  ~CurveProcessor() override = default;

  void saveState(juce::ValueTree &nodeTree) override {
    for (size_t i = 0; i < model.nodes.size(); i++) {
      const auto &node = model.nodes[i];
      juce::Identifier id{"pt" + std::to_string(i)};
      nodeTree.addChild(node->toValueTree(id), -1, nullptr);
    }
  }

  void restoreState(const juce::ValueTree &nodeTree) override {
    model.fromValueTree(nodeTree);
  }

  juce::Component *createEditor(const GraphViewTheme &theme) override;
};

struct VelocityCurveProcessor : CurveProcessor {

  VelocityCurveProcessor(Graph *graph, const std::string &name, uint32_t n_ins, uint32_t n_outs)
    : CurveProcessor(graph, name, n_ins, n_outs, ce::CurveEditorModel<float>(0.0f, 127.0f, 0.0f, 127.0f)) {}

  explicit VelocityCurveProcessor(Graph *graph) :
    CurveProcessor(graph, ce::CurveEditorModel<float>(0.0f, 127.0f, 0.0f, 127.0f)) {}

  void
  on_data(Graph *graph, const std::optional<const Graph::Node::Pin> &pin, Data &data) override {
    juce::ignoreUnused(pin);
    auto input = std::any_cast<juce::MidiBuffer &>(data);
    juce::MidiBuffer output;
    for (auto m: input) {
      auto message = m.getMessage();
      auto velocity = message.getVelocity(); // [0..127]
      auto computed = model.compute(velocity);
      auto scaled = computed / 127.0f;
      scaled = std::min(scaled, 1.0f);
      message.setVelocity(scaled);
      output.addEvent(message, m.samplePosition);
    }
    Data result = std::make_any<juce::MidiBuffer &>(output);
    for (auto &[_, p]: m_outs) {
      p.on_data(graph, result);
    }
  }

  [[nodiscard]] std::string typeId() const override {
    return Processors::velocityCurveProcessor;
  }

  NodeProcessor *clone() override {
    auto c =
      new VelocityCurveProcessor(
        m_graph,
        m_name,
        static_cast<std::uint32_t>(m_ins.size()),
        static_cast<std::uint32_t>(m_outs.size()));
    juce::ValueTree state{"state"};
    saveState(state);
    c->model.fromValueTree(state);
    return c;
  }

};
