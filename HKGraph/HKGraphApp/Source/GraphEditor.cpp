#include "GraphEditor.h"

juce::Component* SumProcessor::createEditor(const GraphViewTheme &theme) {
  return new LabelPanel(this, theme, this);
}

juce::Component* MonitorProcessor::createEditor(const GraphViewTheme &theme) {
  return new LabelPanel(this, theme, this);
}

juce::Component* TransposeProcessor::createEditor(const GraphViewTheme &theme) {
  return new SliderPanel(this, theme);
}

juce::Component* KeyboardPanelProcessor::createEditor(const GraphViewTheme &theme) {
  return new KeyboardPanel(this, theme);
}