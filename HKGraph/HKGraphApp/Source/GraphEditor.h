#pragma once

#include "JuceHeader.h"
#include "Graph.h"
#include "GraphViewComponent.h"
#include "NodeProcessor.h"

struct PassthroughProcessor : public NodeProcessor {

  PassthroughProcessor(Graph *graph, const std::string &name, uint32_t n_ins, uint32_t n_outs)
    : NodeProcessor(graph, name, n_ins, n_outs) {}

  ~PassthroughProcessor() override = default;

  void
  on_data(Graph *graph, const std::optional<const Node::Pin> &pin, Data &data) override {
    juce::ignoreUnused(pin);
    for (auto &[_, p]: m_outs) {
      p.on_data(graph, data);
    }
  }

  [[nodiscard]] std::string typeId() const override {
    return "";
  }

  NodeProcessor *clone() override {
    return new PassthroughProcessor(
      m_graph,
      m_name,
      static_cast<uint32_t>(m_ins.size()),
      static_cast<uint32_t>(m_outs.size()));
  }

  juce::Component *createEditor(const GraphViewTheme &theme) override {
    juce::ignoreUnused(theme);
    return nullptr;
  }
};

struct IntRangeParameter {
  struct Listener {
    virtual void on_parameter_changed(int value) = 0;
  };

  IntRangeParameter(int min, int max, int step, int value)
    : m_min(min), m_max(max), m_step(step), m_value(value) {
  }

  void add_listener(Listener *listener) {
    m_listeners.add(listener);
  }

  void remove_listener(Listener *listener) {
    m_listeners.remove(listener);
  }

  void set_value(int value, bool notify) {
    this->m_value = value;
    if (notify) {
      m_listeners.call([&](Listener &l) { l.on_parameter_changed(value); });
    }
  }

  [[nodiscard]] int get_value() const {
    return m_value;
  }

  int m_min, m_max, m_step;
private:
  int m_value;
  juce::ListenerList<Listener> m_listeners;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IntRangeParameter)
};

struct SliderBinding : public juce::Slider::Listener, public IntRangeParameter::Listener {
  juce::Slider &m_slider;
  IntRangeParameter *m_parameter;

  SliderBinding(juce::Slider &slider, IntRangeParameter *parameter)
    : m_slider(slider), m_parameter(parameter) {
    m_slider.setRange(m_parameter->m_min, m_parameter->m_max, m_parameter->m_step);
    m_slider.setValue(m_parameter->get_value());
    m_slider.addListener(this);
    m_parameter->add_listener(this);
  }

  ~SliderBinding() override {
    m_slider.removeListener(this);
    m_parameter->remove_listener(this);
  }

  void sliderValueChanged(juce::Slider *slider) override {
    auto slider_value = static_cast<int>(slider->getValue());
    auto parameter_value = m_parameter->get_value();
    if (slider_value != parameter_value) {
      m_parameter->set_value(static_cast<int>(slider->getValue()), true);
    }
  }

  void on_parameter_changed(int value) override {
    auto slider_value = static_cast<int>(m_slider.getValue());
    if (slider_value != value) {
      m_slider.setValue(value, juce::NotificationType::dontSendNotification);
    }
  }
};

struct RangeProcessor : public NodeProcessor, public IntRangeParameter::Listener {
  IntRangeParameter *m_parameter;

  RangeProcessor(
    Graph *graph,
    const std::string &name,
    uint32_t n_ins,
    uint32_t n_outs,
    IntRangeParameter *parameter)
    : NodeProcessor(graph, name, n_ins, n_outs),
      m_parameter(parameter) {
    m_parameter->add_listener(this);
  }

  ~RangeProcessor() override {
    m_parameter->remove_listener(this);
    delete m_parameter;
  }

  void on_parameter_changed(int value) override {
    auto data = std::make_any<int>(value);
    for (auto &[_, p]: m_outs) {
      p.async_dispatch(m_graph, data);
    }
  }
};

struct TransposeProcessor : public RangeProcessor {

  TransposeProcessor(Graph *graph, const std::string &name, uint32_t n_ins, uint32_t n_outs)
    : RangeProcessor(graph, name, n_ins, n_outs, new IntRangeParameter(-24, 24, 1, 0)) {
  }

  ~TransposeProcessor() override = default;

  void
  on_data(Graph *graph, const std::optional<const Graph::Node::Pin> &pin, Data &data) override {
    juce::ignoreUnused(graph, pin);
    auto v = std::any_cast<int>(data);
    m_parameter->set_value(v, true); // triggers on_value_changed, which routes the value to m_outs
  }

  [[nodiscard]] std::string typeId() const override {
    return "";
  }

  NodeProcessor *clone() override {
    auto c =
      new TransposeProcessor(
        m_graph,
        m_name,
        static_cast<std::uint32_t>(m_ins.size()),
        static_cast<std::uint32_t>(m_outs.size()));
    c->m_parameter->set_value(this->m_parameter->get_value(), false);
    return c;
  }

  juce::Component *createEditor(const GraphViewTheme &theme) override;

};

template <typename ValueType>
struct ValueProvider {

  struct Listener {
    virtual void on_value(const ValueType &value) = 0;
  };

  ValueProvider() = default;

  void add_listener(Listener* listener) {
    m_listeners.add(listener);
  }

  void remove_listener(Listener* listener) {
    m_listeners.remove(listener);
  }

  void notify(const ValueType &value) {
    m_listeners.call([&](Listener &l) { l.on_value(value); });
  }

private:
  juce::ListenerList<Listener> m_listeners;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ValueProvider)
};

struct SumProcessor : public NodeProcessor, public ValueProvider<int> {
  std::unordered_map<uuid, int> values;

  SumProcessor(Graph *graph, const std::string &name, uint32_t n_ins, uint32_t n_outs)
    : NodeProcessor(graph, name, n_ins, n_outs), ValueProvider<int>() {}

  ~SumProcessor() override = default;

  void on_data(Graph *graph, const std::optional<const Node::Pin> &pin, Data &data) override {
    juce::ignoreUnused(pin);
    if (pin != std::nullopt) {
      values[pin.value().m_id] = std::any_cast<int>(data);
    }
    int sum = std::accumulate(std::begin(values), std::end(values), 0, [&](auto acc, auto &next) {
      return acc + next.second;
    });
    notify(sum);
    Data output = std::make_any<int>(sum);
    for (auto &[_, p]: m_outs) {
      p.on_data(graph, output);
    }
  }

  [[nodiscard]] std::string typeId() const override {
    return "";
  }

  NodeProcessor *clone() override {
    return new SumProcessor(
      m_graph,
      m_name,
      static_cast<uint32_t>(m_ins.size()),
      static_cast<uint32_t>(m_outs.size()));
  }

  juce::Component *createEditor(const GraphViewTheme &theme) override;

};

struct MonitorProcessor : public NodeProcessor, public ValueProvider<int> {

  MonitorProcessor(Graph *graph, const std::string &name, uint32_t n_ins, uint32_t n_outs)
    : NodeProcessor(graph, name, n_ins, n_outs), ValueProvider<int>() {}

  ~MonitorProcessor() override = default;

  void on_data(Graph *graph, const std::optional<const Node::Pin> &pin, Data &data) override {
    juce::ignoreUnused(pin);
    auto input = std::any_cast<int>(data);
    notify(input);
    for (auto &[_, p]: m_outs) {
      p.on_data(graph, data);
    }
  }

  [[nodiscard]] std::string typeId() const override {
    return "";
  }

  NodeProcessor *clone() override {
    return new MonitorProcessor(
      m_graph,
      m_name,
      static_cast<uint32_t>(m_ins.size()),
      static_cast<uint32_t>(m_outs.size()));
  }

  juce::Component *createEditor(const GraphViewTheme &theme) override;

};

struct LabelPanel : public juce::Component, public ValueProvider<int>::Listener {
  NodeProcessor *m_processor;
  GraphViewTheme m_theme;
  ValueProvider<int> *m_value_provider;
  juce::Label m_label;

  LabelPanel(NodeProcessor *p, const GraphViewTheme &viewTheme, ValueProvider<int> *valueProvider)
    : m_processor(p),
      m_theme(viewTheme),
      m_value_provider(valueProvider) {
    m_label.setText("0", juce::NotificationType::dontSendNotification);
    addAndMakeVisible(m_label);
    m_value_provider->add_listener(this);
  }

  ~LabelPanel() override {
    m_value_provider->remove_listener(this);
  }

  void paint(juce::Graphics &g) override {
    g.fillAll(juce::Colour(m_theme.cNodeBackground));
  }

  void resized() override {
    auto bounds = getLocalBounds();
    auto w = static_cast<int>(bounds.toFloat().getWidth() * 0.9f);
    auto h = static_cast<int>(bounds.toFloat().getHeight() * 0.9f);
    m_label.setJustificationType(juce::Justification::centred);
    m_label.centreWithSize(w, h);
  }

  void on_value(const int &value) override {
    m_label.setText(juce::String(value), juce::NotificationType::dontSendNotification);
  }

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LabelPanel)
};

struct SliderPanel : public juce::Component {
  RangeProcessor *processor;
  GraphViewTheme theme;
  juce::Slider slider;
  SliderBinding sliderBinding;

  SliderPanel(TransposeProcessor *p, const GraphViewTheme &viewTheme)
    : processor(p),
      theme(viewTheme),
      slider(juce::Slider::Rotary, juce::Slider::TextBoxBelow),
      sliderBinding(slider, processor->m_parameter) {
    addAndMakeVisible(slider);
  }

  ~SliderPanel() override = default;

  void paint(juce::Graphics &g) override {
    g.fillAll(juce::Colour(theme.cNodeBackground));
  }

  void resized() override {
    auto bounds = getLocalBounds();
    slider.setRange(1.0f, 16.0f, 1.0f);
    auto w = static_cast<int>(bounds.toFloat().getWidth() * 0.9f);
    auto h = static_cast<int>(bounds.toFloat().getHeight() * 0.9f);
    slider.centreWithSize(w, h);
  }

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SliderPanel)
};

struct KeyboardPanelProcessor : public NodeProcessor {
  juce::MidiKeyboardState state{};

  KeyboardPanelProcessor(Graph *graph, const std::string &name, uint32_t n_ins, uint32_t n_outs)
    : NodeProcessor(graph, name, n_ins, n_outs) {}

  ~KeyboardPanelProcessor() override = default;

  void
  on_data(Graph *graph, const std::optional<const Graph::Node::Pin> &pin, Data &data) override {
    juce::ignoreUnused(pin);
    for (auto &[_, p]: m_outs) {
      p.async_dispatch(graph, data);
    }
  }

  [[nodiscard]] std::string typeId() const override {
    return "";
  }

  NodeProcessor *clone() override {
    return new KeyboardPanelProcessor(
      m_graph,
      m_name,
      static_cast<std::uint32_t>(m_ins.size()),
      static_cast<std::uint32_t>(m_outs.size()));
  }

  juce::Component *createEditor(const GraphViewTheme &theme) override;
};

struct KeyboardPanel : public juce::Component {
  KeyboardPanelProcessor *processor;
  GraphViewTheme theme;
  juce::MidiKeyboardComponent keyboardComponent;

  KeyboardPanel(KeyboardPanelProcessor *p, const GraphViewTheme &viewTheme)
    : processor(p),
      theme(viewTheme),
      keyboardComponent(p->state, juce::MidiKeyboardComponent::Orientation::horizontalKeyboard) {
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
    m.addItem(1, "1 x 1");
    m.addItem(2, "2 x 2");
    m.addItem(3, "3 x 3");
    m.addItem(4, "slider");
    m.addItem(5, "sum");
    m.addItem(6, "monitor");
    auto selection = [&](int result) {
      auto position = getMouseXYRelative().toFloat();
      switch (result) {
        case 1:
          addNode(new PassthroughProcessor(graph, "1 x 1", 1, 1), position);
          break;
        case 2:
          addNode(new PassthroughProcessor(graph, "2 x 2", 2, 2), position);
          break;
        case 3:
          addNode(new PassthroughProcessor(graph, "3 x 3", 3, 3), position);
          break;
        case 4:
          addHostNode(new TransposeProcessor(graph, "slider", 1, 1), 150, 150, position);
          break;
        case 5:
          addHostNode(new SumProcessor(graph, "sum", 5, 1), 150, 60, position);
          break;
        case 6:
          addHostNode(new MonitorProcessor(graph, "monitor", 1, 1), 150, 60, position);
          break;
        default:
          break;
      }
    };
    m.showMenuAsync(juce::PopupMenu::Options().withMousePosition(), selection);
  }
};
