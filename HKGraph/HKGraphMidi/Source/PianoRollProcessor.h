#pragma once

#include "Processors.h"
#include "NodeProcessor.h"
#include "RangeParameter.h"
#include "ConstrainedComponent.h"
#include "PianoRollComponent.h"

struct PianoRollProcessor : public NodeProcessor {

  static constexpr int MIN_WIDTH = 600;
  static constexpr int MIN_HEIGHT = 400;

  explicit PianoRollProcessor(Graph *graph) :
    NodeProcessor(graph) {
  }

  PianoRollProcessor(Graph *graph, const std::string &name, uint32_t n_ins, uint32_t n_outs)
    : NodeProcessor(graph, name, n_ins, n_outs) {
  }

  ~PianoRollProcessor() override = default;

  void
  on_data(Graph *graph, const std::optional<const Graph::Node::Pin> &pin, Data &data) override {
    juce::ignoreUnused(graph, pin, data);
  }

  [[nodiscard]] std::string typeId() const override {
    return Processors::pianoRollProcessor;
  }

  void saveState(juce::ValueTree &nodeTree) override {
    juce::ignoreUnused(nodeTree);
  }

  void restoreState(const juce::ValueTree &nodeTree) override {
    juce::ignoreUnused(nodeTree);
  }

  NodeProcessor *clone() override {
    auto c =
      new PianoRollProcessor(
        m_graph,
        m_name,
        static_cast<std::uint32_t>(m_ins.size()),
        static_cast<std::uint32_t>(m_outs.size()));
    c->m_muted = m_muted;
    return c;
  }

  juce::Component *createEditor(const GraphViewTheme &theme) override;

  struct Panel : public ConstrainedComponent {
    PianoRollProcessor *processor;
    GraphViewTheme theme;
    juce::Viewport pianoRollViewPort;
    NoteGridComponent noteGridComponent;
    juce::Slider sliderNoteGridLaneHeight;
    juce::Slider sliderNoteGridBarWidth;

    Panel(PianoRollProcessor *p, const GraphViewTheme &viewTheme)
      : ConstrainedComponent(),
        processor(p),
        theme(viewTheme),
        sliderNoteGridLaneHeight(juce::Slider::LinearBar, juce::Slider::NoTextBox),
        sliderNoteGridBarWidth(juce::Slider::LinearBar, juce::Slider::NoTextBox){
      m_constrains.setMinimumSize(200, 200);

      noteGridComponent.setSize(
        noteGridComponent.bars * noteGridComponent.barWidth,
        127 * noteGridComponent.laneHeight);

      pianoRollViewPort.setViewedComponent(&noteGridComponent, false);
      addAndMakeVisible(pianoRollViewPort);

      sliderNoteGridLaneHeight.setRange(noteGridComponent.laneHeight, 128, 1);
      sliderNoteGridLaneHeight.setTextBoxIsEditable(false);
      sliderNoteGridLaneHeight.onValueChange = [this]() {
        noteGridComponent.laneHeight = static_cast<int>(sliderNoteGridLaneHeight.getValue());
        noteGridComponent.setSize(
          noteGridComponent.bars * noteGridComponent.barWidth,
          127 * noteGridComponent.laneHeight);
        noteGridComponent.repaint();
      };
      addAndMakeVisible(sliderNoteGridLaneHeight);

      sliderNoteGridBarWidth.setRange(noteGridComponent.barWidth, 512, 8);
      sliderNoteGridBarWidth.setTextBoxIsEditable(false);
      sliderNoteGridBarWidth.onValueChange = [this]() {
        noteGridComponent.barWidth = static_cast<int>(sliderNoteGridBarWidth.getValue());
        noteGridComponent.setSize(
          noteGridComponent.bars * noteGridComponent.barWidth,
          127 * noteGridComponent.laneHeight);
        noteGridComponent.repaint();
      };
      addAndMakeVisible(sliderNoteGridBarWidth);
    }

    ~Panel() override = default;

    void paint(juce::Graphics &g) override {
      g.fillAll(juce::Colour(theme.cNodeBackground));
    }

    void resized() override {
      auto bounds = getLocalBounds().reduced(10, 10);

      juce::FlexBox fb;
      fb.flexDirection = juce::FlexBox::Direction::column;
      fb.justifyContent = juce::FlexBox::JustifyContent::center;
      fb.alignContent = juce::FlexBox::AlignContent::stretch;
      fb.flexWrap = juce::FlexBox::Wrap::noWrap;
      fb.alignItems = juce::FlexBox::AlignItems::stretch;

      juce::FlexItem::Margin margin(
        0.0f, // top
        0.0f, // right
        5.0f, // bottom
        0.0f  // left
      );

      fb.items.add(
        juce::FlexItem(pianoRollViewPort)
          .withMinHeight(200.0f)
          .withFlex(0.8f)
          .withMargin(margin));

      juce::FlexBox controls;
      controls.flexDirection = juce::FlexBox::Direction::row;
      controls.justifyContent = juce::FlexBox::JustifyContent::flexStart;
      controls.alignContent = juce::FlexBox::AlignContent::flexStart;
      controls.flexWrap = juce::FlexBox::Wrap::wrap;
      controls.alignItems = juce::FlexBox::AlignItems::flexStart;

      controls.items.add(
        juce::FlexItem(sliderNoteGridLaneHeight)
          .withMinHeight(10.0f)
          .withMaxHeight(10.0f)
          .withMinWidth(40.0f)
          .withMaxWidth(40.0f)
          .withFlex(0.2f)
          .withMargin(margin));
      controls.items.add(
        juce::FlexItem(sliderNoteGridBarWidth)
          .withMinHeight(10.0f)
          .withMaxHeight(10.0f)
          .withMinWidth(40.0f)
          .withMaxWidth(40.0f)
          .withFlex(0.2f)
          .withMargin(margin));

      fb.items.add(
        juce::FlexItem(controls)
          .withFlex(0.2f));

      fb.performLayout(bounds);
    }

  private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Panel)
  };

};
