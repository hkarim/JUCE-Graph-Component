#pragma once

#include "../Processors.h"
#include "NodeProcessor.h"
#include "../RangeParameter.h"
#include "ConstrainedComponent.h"
#include "PianoRollComponent.h"

struct PianoRollProcessor : public NodeProcessor {

  static constexpr int MIN_WIDTH = 600;
  static constexpr int MIN_HEIGHT = 600;

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
    PianoRollComponent pianoRoll;
    juce::Slider sliderNoteGridLaneHeight;
    juce::Slider sliderNoteGridBarWidth;
    juce::Slider sliderNoteGridQuantize;

    juce::Slider sliderTimeSignatureNumerator;
    juce::Slider sliderTimeSignatureDenominator;

    juce::Label labelTimeSignature{};

    Panel(PianoRollProcessor *p, const GraphViewTheme &viewTheme)
      : ConstrainedComponent(),
        processor(p),
        theme(viewTheme),
        sliderNoteGridLaneHeight(juce::Slider::LinearBar, juce::Slider::NoTextBox),
        sliderNoteGridBarWidth(juce::Slider::LinearBar, juce::Slider::NoTextBox),
        sliderNoteGridQuantize(juce::Slider::LinearBar, juce::Slider::NoTextBox),
        sliderTimeSignatureNumerator(juce::Slider::LinearBar, juce::Slider::NoTextBox),
        sliderTimeSignatureDenominator(juce::Slider::LinearBar, juce::Slider::NoTextBox) {
      m_constrains.setMinimumSize(200, 200);

      addAndMakeVisible(pianoRoll);

      sliderNoteGridLaneHeight.setRange(1.0f, 4.0f, 0.1f);
      sliderNoteGridLaneHeight.setValue(1.0f);
      sliderNoteGridBarWidth.setRange(0.3f, 4.0f, 0.1f);
      sliderNoteGridBarWidth.setValue(1.0f);
      auto onScaleChange = [this]() {
        auto xp = static_cast<float>(sliderNoteGridBarWidth.getValue());
        auto yp = static_cast<float>(sliderNoteGridLaneHeight.getValue());
        pianoRoll.setScale(xp, yp);
      };

      sliderNoteGridLaneHeight.setTextBoxIsEditable(false);
      sliderNoteGridLaneHeight.onValueChange = onScaleChange;
      sliderNoteGridBarWidth.setTextBoxIsEditable(false);
      sliderNoteGridBarWidth.onValueChange = onScaleChange;

      addAndMakeVisible(sliderNoteGridLaneHeight);
      addAndMakeVisible(sliderNoteGridBarWidth);

      sliderNoteGridQuantize.setRange(0, 6, 1);
      sliderNoteGridQuantize.setValue(4);
      sliderNoteGridQuantize.setTextBoxIsEditable(false);
      sliderNoteGridQuantize.onValueChange = [this]() {
        auto v = static_cast<int>(sliderNoteGridQuantize.getValue());
        auto q = static_cast<int>(std::pow(2, v));
        pianoRoll.setQuantize(q);
        pianoRoll.repaint();
      };
      addAndMakeVisible(sliderNoteGridQuantize);

      // Time Signature
      auto onTimeSignatureChange = [this]() {
        auto n = static_cast<int>(sliderTimeSignatureNumerator.getValue());
        auto d = static_cast<int>(sliderTimeSignatureDenominator.getValue());
        d = static_cast<int>(std::pow(2, d));
        pianoRoll.setTimeSignature(n, d);
        labelTimeSignature.setText(juce::String(n) + "/" + juce::String(d), juce::NotificationType::dontSendNotification);
        pianoRoll.repaint();
      };
      sliderTimeSignatureNumerator.setRange(1, 32, 1);
      sliderTimeSignatureNumerator.setValue(4.0f);
      sliderTimeSignatureNumerator.setTextBoxIsEditable(false);
      sliderTimeSignatureNumerator.onValueChange = onTimeSignatureChange;
      addAndMakeVisible(sliderTimeSignatureNumerator);

      sliderTimeSignatureDenominator.setRange(1, 5, 1); // 2's exponent
      sliderTimeSignatureDenominator.setValue(2); // 2^2
      sliderTimeSignatureDenominator.setTextBoxIsEditable(false);
      sliderTimeSignatureDenominator.onValueChange = onTimeSignatureChange;
      addAndMakeVisible(sliderTimeSignatureDenominator);

      addAndMakeVisible(labelTimeSignature);
    }

    ~Panel() override = default;

    void paint(juce::Graphics &g) override {
      g.fillAll(juce::Colour(theme.cNodeBackground));
      //g.fillAll(juce::Colour(0x373A3E));
    }

    void resized() override {
      auto bounds = getLocalBounds();//.reduced(10, 10);

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
        1.0f  // left
      );

      fb.items.add(
        juce::FlexItem(pianoRoll)
          .withMinHeight(200.0f)
          .withFlex(1.0f)
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
      controls.items.add(
        juce::FlexItem(sliderNoteGridQuantize)
          .withMinHeight(10.0f)
          .withMaxHeight(10.0f)
          .withMinWidth(40.0f)
          .withMaxWidth(40.0f)
          .withFlex(0.2f)
          .withMargin(margin));
      controls.items.add(
        juce::FlexItem(sliderTimeSignatureNumerator)
          .withAlignSelf(juce::FlexItem::AlignSelf::flexEnd)
          .withMinHeight(10.0f)
          .withMaxHeight(10.0f)
          .withMinWidth(40.0f)
          .withMaxWidth(40.0f)
          .withFlex(0.2f)
          .withMargin(margin));
      controls.items.add(
        juce::FlexItem(sliderTimeSignatureDenominator)
          .withAlignSelf(juce::FlexItem::AlignSelf::flexEnd)
          .withMinHeight(10.0f)
          .withMaxHeight(10.0f)
          .withMinWidth(40.0f)
          .withMaxWidth(40.0f)
          .withFlex(0.2f)
          .withMargin(margin));
      controls.items.add(
        juce::FlexItem(labelTimeSignature)
          .withAlignSelf(juce::FlexItem::AlignSelf::flexEnd)
          .withMinHeight(10.0f)
          .withMaxHeight(10.0f)
          .withMinWidth(40.0f)
          .withMaxWidth(40.0f)
          .withFlex(0.2f)
          .withMargin(margin));



      fb.items.add(
        juce::FlexItem(controls)
          .withMinHeight(10.0f)
          .withMargin(margin));

      fb.performLayout(bounds);
    }

  private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Panel)
  };

};
