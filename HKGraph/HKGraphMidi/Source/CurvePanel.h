#include "CurveProcessor.h"
#include "ConstrainedComponent.h"

struct CurvePanel : public ConstrainedComponent {
  CurveProcessor *processor;
  GraphViewTheme theme;
  ce::CurveEditor<float> editor;

  CurvePanel(CurveProcessor *p, const GraphViewTheme &viewTheme)
    : ConstrainedComponent(),
      processor(p),
      theme(viewTheme),
      editor(p->model) {
    m_constrains.setMinimumSize(200, 200);
    addAndMakeVisible(editor);
  }

  ~CurvePanel() override = default;

  void paint(juce::Graphics &g) override {
    g.fillAll(juce::Colour(theme.cNodeBackground));
  }

  void resized() override {
    auto bounds = getLocalBounds().reduced(10, 10);
    auto w = static_cast<int>(bounds.toFloat().getWidth());
    auto h = static_cast<int>(bounds.toFloat().getHeight());
    editor.centreWithSize(w, h);
  }

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CurvePanel)
};

struct MidiControllerCurvePanel : public CurvePanel, private juce::Slider::Listener {
  juce::Slider sliderController;

  MidiControllerCurvePanel(ControllerCurveProcessor *p, const GraphViewTheme &viewTheme)
    : CurvePanel(p, viewTheme),
      sliderController(juce::Slider::LinearBar, juce::Slider::TextBoxLeft) {
    m_constrains.setMinimumSize(280, 280);
    sliderController.setRange(0.0f, 127.0f, 1.0f);
    sliderController.addListener(this);
    sliderController.textFromValueFunction = [](double v) -> juce::String {
      auto n = static_cast<int>(v);
      return juce::String("CC#") + juce::String(n);
    };
    sliderController.setValue(p->controllerType);
    addAndMakeVisible(sliderController);
  }

  ~MidiControllerCurvePanel() override {
    sliderController.removeListener(this);
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
      juce::FlexItem(sliderController)
        .withMinHeight(20.0f)
        .withMaxHeight(20.0f)
        .withFlex(0.2f)
        .withMargin(margin));
    fb.items.add(
      juce::FlexItem(editor)
        .withMinWidth(200.0f)
        .withMinHeight(200.0f)
        .withFlex(0.8f));

    fb.performLayout(bounds);
  }

  void sliderValueChanged(juce::Slider *slider) override {
    if (auto *p = dynamic_cast<ControllerCurveProcessor *>(processor)) {
      p->controllerType = static_cast<int>(slider->getValue());
    }
  }

};

