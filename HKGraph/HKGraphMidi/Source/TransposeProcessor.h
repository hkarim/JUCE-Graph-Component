#pragma once
#include "Processors.h"
#include "NodeProcessor.h"
#include "RangeParameter.h"

struct TransposeProcessor : public NodeProcessor {
  IntRangeParameter m_parameter;

  explicit TransposeProcessor(Graph *graph) :
    NodeProcessor(graph),
    m_parameter(-24, 24, 1, 0) {
  }

  TransposeProcessor(Graph *graph, const std::string &name, uint32_t n_ins, uint32_t n_outs)
    : NodeProcessor(graph, name, n_ins, n_outs),
      m_parameter(-24, 24, 1, 0) {
  }

  ~TransposeProcessor() override = default;

  void
  on_data(Graph *graph, const std::optional<const Graph::Node::Pin> &pin, Data &data) override {
    juce::ignoreUnused(pin);
    auto shift = m_parameter.value;

    auto input = std::any_cast<juce::MidiBuffer &>(data);
    juce::MidiBuffer output;
    /////
    if (m_parameter.changed) {
      for (auto i = 1; i <= 16; i++) {
        output.addEvent(juce::MidiMessage::allNotesOff(i), 0);
      }
      m_parameter.changed = false;
    }
    /////
    if (input.getNumEvents() > 0) {
      for (auto m: input) {
        auto message = m.getMessage();
        if (message.isNoteOn()) {
          output.addEvent(
            juce::MidiMessage::noteOn(
              message.getChannel(),
              message.getNoteNumber() + shift,
              message.getFloatVelocity()),
            m.samplePosition
          );
        } else if (message.isNoteOff()) {
          output.addEvent(
            juce::MidiMessage::noteOff(
              message.getChannel(),
              message.getNoteNumber() + shift),
            m.samplePosition
          );
        } else {
          output.addEvent(m.getMessage(), m.samplePosition);
        }
      }

    }
    Data result = std::make_any<juce::MidiBuffer &>(output);
    for (auto &[_, p]: m_outs) {
      p.async_dispatch(graph, result);
    }
  }

  [[nodiscard]] std::string typeId() const override {
    return Processors::transposeProcessor;
  }

  void saveState(juce::ValueTree& nodeTree) override {
    nodeTree.setProperty("TransposeProcessor::parameter", juce::var(m_parameter.value), nullptr);
  }

  void restoreState(const juce::ValueTree& nodeTree) override {
    m_parameter.value = nodeTree.getProperty("TransposeProcessor::parameter");
  }

  NodeProcessor *clone() override {
    auto c =
      new TransposeProcessor(
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
