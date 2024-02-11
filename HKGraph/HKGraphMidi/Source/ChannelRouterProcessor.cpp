#include "ChannelRouterProcessor.h"
#include "GraphEditor.h"

juce::Component* ChannelRouterProcessor::createEditor(const GraphViewTheme &theme) {
  return new SliderPanel(this, theme, m_parameter);
}


