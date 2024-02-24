#pragma once

#include "JuceHeader.h"
#include "Graph.h"
#include "GraphViewComponent.h"
#include "NodeProcessor.h"
#include "MidiInNodeProcessor.h"
#include "MidiOutNodeProcessor.h"
#include "ChannelSplitterProcessor.h"
#include "ConstrainedComponent.h"
#include "ChannelRouterProcessor.h"
#include "TransposeProcessor.h"
#include "KeyboardProcessor.h"
#include "CurveProcessor.h"
#include "NoteFilterProcessor.h"
#include "ChordSplitterProcessor.h"
#include "GraphLookAndFeel.h"

struct SliderBinding : public juce::Slider::Listener {
  juce::Slider &m_slider;
  IntRangeParameter &m_parameter;

  SliderBinding(juce::Slider &slider, IntRangeParameter &parameter)
    : m_slider(slider), m_parameter(parameter) {
    m_slider.setRange(m_parameter.min, m_parameter.max, m_parameter.step);
    m_slider.setValue(m_parameter.value);
    m_slider.addListener(this);
  }

  ~SliderBinding() override {
    m_slider.removeListener(this);
  }

  void sliderValueChanged(juce::Slider *slider) override {
    m_parameter.value = static_cast<int>(slider->getValue());
    m_parameter.changed = true;
  }
};

struct SliderPanel : public juce::Component {
  NodeProcessor *m_processor;
  GraphViewTheme m_theme;
  juce::Slider m_slider;
  SliderBinding m_binding;
  GraphLookAndFeel m_laf;

  SliderPanel(NodeProcessor *processor, const GraphViewTheme &theme, IntRangeParameter &parameter)
    : m_processor(processor),
      m_theme(theme),
      m_slider(juce::Slider::Rotary, juce::Slider::TextBoxBelow),
      m_binding(m_slider, parameter) {
    m_slider.setLookAndFeel(&m_laf);
    addAndMakeVisible(m_slider);
  }

  ~SliderPanel() override {
    m_slider.setLookAndFeel(nullptr);
  }

  void paint(juce::Graphics &g) override {
    g.fillAll(juce::Colour(m_theme.cNodeBackground));
  }

  void resized() override {
    auto bounds = getLocalBounds();
    auto w = static_cast<int>(bounds.toFloat().getWidth() * 0.9f);
    auto h = static_cast<int>(bounds.toFloat().getHeight() * 0.9f);
    m_slider.centreWithSize(w, h);
  }

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SliderPanel)
};

struct KeyboardPanel : public ConstrainedComponent {
  KeyboardProcessor *processor;
  GraphViewTheme theme;
  juce::MidiKeyboardComponent keyboardComponent;

  KeyboardPanel(KeyboardProcessor *p, const GraphViewTheme &viewTheme)
    : ConstrainedComponent(),
      processor(p),
      theme(viewTheme),
      keyboardComponent(p->keyboardState, juce::MidiKeyboardComponent::Orientation::horizontalKeyboard) {
    m_constrains.setMinimumSize(400, 100);
    m_constrains.setMaximumHeight(100);
    addAndMakeVisible(keyboardComponent);
  }

  ~KeyboardPanel() override = default;

  void paint(juce::Graphics &g) override {
    g.fillAll(juce::Colour(theme.cNodeBackground));
  }

  void resized() override {
    auto bounds = getLocalBounds();
    keyboardComponent.setBounds(bounds);
  }

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KeyboardPanel)
};

struct GraphEditor : public GraphViewComponent {

  explicit GraphEditor(Graph *g) : GraphViewComponent(g) {
  }

  ~GraphEditor() override = default;

  void popupMenu(const juce::MouseEvent &e) override {
    juce::ignoreUnused(e);
    juce::PopupMenu m;
    m.addItem(1, "transpose");
    m.addItem(2, "channel-splitter");
    m.addItem(3, "channel-router");
    m.addItem(4, "chord-splitter");
    m.addItem(5, "keyboard");
    m.addItem(6, "note-filter");
    m.addItem(7, "velocity-curve");
    m.addItem(8, "cc-curve");
    auto selection = [&](int result) {
      auto position = getMouseXYRelative().toFloat();
      switch (result) {
        case 1:
          addHostNode(new TransposeProcessor(graph, "transpose", 1, 1), 150, 150, position);
          break;
        case 2:
          addNode(new ChannelSplitterProcessor(graph, "channel-splitter", 1, 16), position);
          break;
        case 3:
          addHostNode(new ChannelRouterProcessor(graph, "channel-router", 1, 1), 150, 150, position);
          break;
        case 4:
          addNode(new ChordSplitterProcessor(graph, "chord-splitter", 1, 16), position);
          break;
        case 5:
          addHostNode(new KeyboardProcessor(graph, "keyboard", 1, 1), 400, 100, position);
          break;
        case 6:
          addHostNode(
            new NoteFilterProcessor(graph, "note-filter", 1, 1),
            NoteFilterProcessor::MIN_WIDTH,
            NoteFilterProcessor::MIN_HEIGHT,
            position);
          break;
        case 7:
          addHostNode(new VelocityCurveProcessor(graph, "velocity-curve", 1, 1), 300, 300, position);
          break;
        case 8:
          addHostNode(
            new ControllerCurveProcessor(graph, "cc-curve", 1, 1),
            ControllerCurveProcessor::MIN_WIDTH,
            ControllerCurveProcessor::MIN_HEIGHT,
            position);
          break;
        default:
          break;
      }
    };
    m.showMenuAsync(juce::PopupMenu::Options().withMousePosition(), selection);
  }
};
