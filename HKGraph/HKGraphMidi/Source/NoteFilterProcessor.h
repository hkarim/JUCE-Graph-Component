#pragma once

#include "Processors.h"
#include "KeyboardProcessor.h"
#include "ConstrainedComponent.h"

struct GraphKeyboardModel {
  int lastNotePressed{-1};
  bool showAllNoteNames{};
  std::vector<int> disabledNoteNumbers;
  int enabledRangeBegin{1};
  int enabledRangeEnd{127};
};

struct GraphKeyboardComponent : public juce::MidiKeyboardComponent {
  GraphKeyboardModel &model;

  GraphKeyboardComponent(
    juce::MidiKeyboardState &state,
    juce::MidiKeyboardComponent::Orientation orientation,
    GraphKeyboardModel &graphKeyboardModel) :
    juce::MidiKeyboardComponent(state, orientation),
    model(graphKeyboardModel) {}

  juce::String getWhiteNoteText(int midiNoteNumber) override {
    if (model.showAllNoteNames) {
      return juce::MidiMessage::getMidiNoteName(midiNoteNumber, true, true, getOctaveForMiddleC());
    }
    return juce::MidiKeyboardComponent::getWhiteNoteText(midiNoteNumber);
  }

  void drawWhiteNote(int midiNoteNumber, juce::Graphics &g, juce::Rectangle<float> area, bool isDown, bool isOver,
                     juce::Colour lineColour, juce::Colour textColour) override {
    MidiKeyboardComponent::drawWhiteNote(midiNoteNumber, g, area, isDown, isOver, lineColour, textColour);
    auto p = std::find(model.disabledNoteNumbers.begin(), model.disabledNoteNumbers.end(), midiNoteNumber);
    if (p != std::end(model.disabledNoteNumbers)) {
      g.setColour(juce::Colours::lightpink);
      g.fillRect(area);
      if (!lineColour.isTransparent()) {
        g.setColour(lineColour);
        auto orientation = getOrientation();
        switch (orientation) {
          case horizontalKeyboard:
            g.fillRect(area.withWidth(1.0f));
            break;
          case verticalKeyboardFacingLeft:
            g.fillRect(area.withHeight(1.0f));
            break;
          case verticalKeyboardFacingRight:
            g.fillRect(area.removeFromBottom(1.0f));
            break;
          default:
            break;
        }
        if (midiNoteNumber == getRangeEnd()) {
          switch (orientation) {
            case horizontalKeyboard:
              g.fillRect(area.expanded(1.0f, 0).removeFromRight(1.0f));
              break;
            case verticalKeyboardFacingLeft:
              g.fillRect(area.expanded(0, 1.0f).removeFromBottom(1.0f));
              break;
            case verticalKeyboardFacingRight:
              g.fillRect(area.expanded(0, 1.0f).removeFromTop(1.0f));
              break;
            default:
              break;
          }
        }
      }
    }
  }

  void drawBlackNote(int midiNoteNumber, juce::Graphics &g, juce::Rectangle<float> area, bool isDown, bool isOver,
                     juce::Colour noteFillColour) override {
    MidiKeyboardComponent::drawBlackNote(midiNoteNumber, g, area, isDown, isOver, noteFillColour);
    auto p = std::find(model.disabledNoteNumbers.begin(), model.disabledNoteNumbers.end(), midiNoteNumber);
    if (p != std::end(model.disabledNoteNumbers)) {
      g.setColour(juce::Colours::red);
      g.fillRect(area);
    }
  }

  bool mouseDownOnKey(int midiNoteNumber, const juce::MouseEvent &e) override {
    model.lastNotePressed = midiNoteNumber;
    if (e.mods.isAnyModifierKeyDown()) {
      toggleKey(midiNoteNumber);
    }
    return isEnabled(midiNoteNumber);
  }

  bool mouseDraggedToKey(int midiNoteNumber, const juce::MouseEvent &e) override {
    if (e.mods.isAnyModifierKeyDown()) {
      std::vector<int> range(
        static_cast<std::vector<int>::size_type>(std::abs(model.lastNotePressed - midiNoteNumber)));
      auto firstNote = std::min(model.lastNotePressed, midiNoteNumber);
      std::iota(std::begin(range), std::end(range), firstNote);
      for (auto candidate: range) {
        toggleKey(candidate);
      }
    }
    model.lastNotePressed = midiNoteNumber;
    return isEnabled(midiNoteNumber);
  }

  void mouseUpOnKey(int /*midiNoteNumber*/, const juce::MouseEvent &) override {
    model.lastNotePressed = -1;
  }

private:

  void toggleKey(int midiNoteNumber) {
    auto p = std::find(std::begin(model.disabledNoteNumbers), std::end(model.disabledNoteNumbers), midiNoteNumber);
    if (p != std::end(model.disabledNoteNumbers)) {
      model.disabledNoteNumbers.erase(p);
    } else {
      model.disabledNoteNumbers.push_back(midiNoteNumber);
    }
  }

  bool isEnabled(int midiNoteNumber) {
    auto p = std::find(std::begin(model.disabledNoteNumbers), std::end(model.disabledNoteNumbers), midiNoteNumber);
    return p == std::end(model.disabledNoteNumbers);
  }

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GraphKeyboardComponent)
};

struct NoteFilterProcessor : public KeyboardProcessor {
  static constexpr int MIN_WIDTH = 400;
  static constexpr int MIN_HEIGHT = 150;

  GraphKeyboardModel model;

  explicit NoteFilterProcessor(Graph *graph) :
    KeyboardProcessor(graph) {
  }

  NoteFilterProcessor(Graph *graph, const std::string &name, uint32_t n_ins, uint32_t n_outs)
    : KeyboardProcessor(graph, name, n_ins, n_outs) {
  }

  void
  on_data(Graph *graph, const std::optional<const Graph::Node::Pin> &pin, Data &data) override {
    juce::ignoreUnused(pin);
    auto input = std::any_cast<Block>(data);
    juce::MidiBuffer output;
    if (!input.midiBuffer.isEmpty()) {
      for (auto const &n: input.midiBuffer) {
        auto message = n.getMessage();
        if (message.isNoteOnOrOff()) {
          auto p = std::find(std::begin(model.disabledNoteNumbers), std::end(model.disabledNoteNumbers),
                             message.getNoteNumber());
          if (p == std::end(model.disabledNoteNumbers)) { // note NOT disabled
            output.addEvent(message, n.samplePosition);
          }
        } else {
          output.addEvent(message, n.samplePosition);
        }
      }
      keyboardState.processNextMidiBuffer(output, 0, output.getNumEvents(), true);
    }
    if (hasCalledReset) {
      keyboardMessageCollector.removeNextBlockOfMessages(output, input.audioBuffer.getNumSamples());
    }
    Data result = std::make_any<Block>(input.audioBuffer, output);
    for (auto &[_, p]: m_outs) {
      p.async_dispatch(graph, result);
    }
  }

  [[nodiscard]] std::string typeId() const override {
    return Processors::noteFilterProcessor;
  }

  void saveState(juce::ValueTree &nodeTree) override {
    juce::ValueTree modelTree{"model-tree"};
    modelTree.setProperty("showAllNoteNames", juce::var(model.showAllNoteNames), nullptr);
    modelTree.setProperty("lastNotePressed", juce::var(model.lastNotePressed), nullptr);
    modelTree.setProperty("enabledRangeBegin", juce::var(model.enabledRangeBegin), nullptr);
    modelTree.setProperty("enabledRangeEnd", juce::var(model.enabledRangeEnd), nullptr);
    juce::Array<juce::var> disabledNoteNumbers;
    disabledNoteNumbers.resize(static_cast<int>(model.disabledNoteNumbers.size()));
    for (auto n: model.disabledNoteNumbers) {
      disabledNoteNumbers.add(juce::var(n));
    }
    modelTree.setProperty("disabledNoteNumbers", juce::var(disabledNoteNumbers), nullptr);

    nodeTree.appendChild(modelTree, nullptr);
  }

  void restoreState(const juce::ValueTree &nodeTree) override {
    auto modelTree = nodeTree.getChildWithName("model-tree");
    model.showAllNoteNames = modelTree.getProperty("showAllNoteNames");
    model.lastNotePressed = modelTree.getProperty("lastNotePressed");
    model.enabledRangeBegin = modelTree.getProperty("enabledRangeBegin");
    model.enabledRangeEnd = modelTree.getProperty("enabledRangeEnd");
    juce::Array<juce::var> *disabledNoteNumbers = modelTree.getProperty("disabledNoteNumbers").getArray();
    model.disabledNoteNumbers.clear();
    for (auto const &v: *disabledNoteNumbers) {
      int n = v;
      model.disabledNoteNumbers.push_back(n);
    }
  }

  NodeProcessor *clone() override {
    auto c = new NoteFilterProcessor(
      m_graph,
      m_name,
      static_cast<std::uint32_t>(m_ins.size()),
      static_cast<std::uint32_t>(m_outs.size()));
    c->m_muted = m_muted;
    c->model = model;
    return c;
  }

  struct KeyboardPanel : public ConstrainedComponent, juce::Slider::Listener {
    NoteFilterProcessor *processor;
    GraphViewTheme theme;
    GraphKeyboardComponent keyboardComponent;
    juce::Slider sliderBeginNoteRange;
    juce::Slider sliderEndNoteRange;

    KeyboardPanel(NoteFilterProcessor *p, const GraphViewTheme &viewTheme)
      : ConstrainedComponent(),
        processor(p),
        theme(viewTheme),
        keyboardComponent(
          p->keyboardState,
          juce::MidiKeyboardComponent::Orientation::horizontalKeyboard,
          p->model),
        sliderBeginNoteRange(juce::Slider::LinearBar, juce::Slider::TextBoxLeft),
        sliderEndNoteRange(juce::Slider::LinearBar, juce::Slider::TextBoxRight) {
      m_constrains.setMinimumSize(NoteFilterProcessor::MIN_WIDTH, NoteFilterProcessor::MIN_HEIGHT);
      m_constrains.setMaximumWidth(750);
      m_constrains.setMaximumHeight(NoteFilterProcessor::MIN_HEIGHT);
      addAndMakeVisible(keyboardComponent);

      auto valueToNoteName = [](double v) -> juce::String {
        auto n = static_cast<int>(v);
        return juce::MidiMessage::getMidiNoteName(n, true, true, 3);
      };

      sliderBeginNoteRange.setRange(0, 127, 1);
      sliderBeginNoteRange.setTextBoxIsEditable(false);
      sliderBeginNoteRange.textFromValueFunction = valueToNoteName;
      sliderBeginNoteRange.setValue(processor->model.enabledRangeBegin);
      sliderBeginNoteRange.addListener(this);
      addAndMakeVisible(sliderBeginNoteRange);

      sliderEndNoteRange.setRange(0, 127, 1);
      sliderEndNoteRange.setTextBoxIsEditable(false);
      sliderEndNoteRange.textFromValueFunction = valueToNoteName;
      sliderEndNoteRange.setValue(processor->model.enabledRangeEnd);
      sliderEndNoteRange.addListener(this);
      addAndMakeVisible(sliderEndNoteRange);
    }

    ~KeyboardPanel() override {
      sliderBeginNoteRange.removeListener(this);
      sliderEndNoteRange.removeListener(this);
    }

    void paint(juce::Graphics &g) override {
      g.fillAll(juce::Colour(theme.cNodeBackground));
    }

    void resized() override {
      juce::FlexBox flexBox;
      flexBox.flexDirection = juce::FlexBox::Direction::row;
      flexBox.justifyContent = juce::FlexBox::JustifyContent::center;
      flexBox.alignContent = juce::FlexBox::AlignContent::stretch;
      flexBox.flexWrap = juce::FlexBox::Wrap::wrap;
      flexBox.alignItems = juce::FlexBox::AlignItems::stretch;

      // keyboard
      flexBox.items.add(
        juce::FlexItem(keyboardComponent)
          .withMinWidth(NoteFilterProcessor::MIN_WIDTH)
          .withMinHeight(60.0f)
          .withMaxHeight(60.0f)
          .withFlex(1.0f));

      // controls
      juce::FlexBox controls;
      controls.flexDirection = juce::FlexBox::Direction::row;
      controls.justifyContent = juce::FlexBox::JustifyContent::flexStart;
      controls.alignContent = juce::FlexBox::AlignContent::flexStart;
      controls.flexWrap = juce::FlexBox::Wrap::wrap;
      controls.alignItems = juce::FlexBox::AlignItems::flexStart;
      juce::FlexItem::Margin sliderMargin(
        2.0f, // top
        2.0f, // right
        5.0f, // bottom
        5.0f  // left
      );
      controls.items.add(
        juce::FlexItem(sliderBeginNoteRange)
          .withMinHeight(30.0f)
          .withMinWidth(100.0f)
          .withFlex(0.4f)
          .withMargin(sliderMargin));
      controls.items.add(
        juce::FlexItem(sliderEndNoteRange)
          .withMinHeight(30.0f)
          .withMinWidth(100.0f)
          .withFlex(0.4f)
          .withMargin(sliderMargin));
      juce::FlexItem::Margin controlsMargin(
        5.0f, // left
        5.0f, // right
        5.0f, // top
        5.0f  // bottom
      );
      flexBox.items.add(
        juce::FlexItem(controls)
          .withMinWidth(NoteFilterProcessor::MIN_WIDTH)
          .withMinHeight(30.0f)
          .withFlex(1.0f)
          .withMargin(controlsMargin));

      auto bounds = getLocalBounds();
      flexBox.performLayout(bounds);
    }

    void sliderValueChanged(juce::Slider *) override {
      processor->model.disabledNoteNumbers.clear();
      processor->model.enabledRangeBegin = static_cast<int>(sliderBeginNoteRange.getValue());
      for (auto i{0}; i < processor->model.enabledRangeBegin; ++i) {
        processor->model.disabledNoteNumbers.push_back(i);
      }
      processor->model.enabledRangeEnd = static_cast<int>(sliderEndNoteRange.getValue());
      for (auto i{processor->model.enabledRangeEnd + 1}; i <= 127; ++i) {
        processor->model.disabledNoteNumbers.push_back(i);
      }
      keyboardComponent.repaint();
    }

  private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KeyboardPanel)
  };


  juce::Component *createEditor(const GraphViewTheme &theme) override {
    return new KeyboardPanel(this, theme);
  }
};
