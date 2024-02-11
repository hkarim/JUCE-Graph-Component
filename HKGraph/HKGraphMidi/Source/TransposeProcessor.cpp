#include "TransposeProcessor.h"
#include "GraphEditor.h"

juce::Component* TransposeProcessor::createEditor(const GraphViewTheme &theme) {
  return new SliderPanel(this, theme, m_parameter);
}

