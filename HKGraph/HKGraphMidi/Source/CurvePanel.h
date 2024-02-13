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

