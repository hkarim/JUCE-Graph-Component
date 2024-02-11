#include "KeyboardProcessor.h"
#include "GraphEditor.h"

juce::Component* KeyboardProcessor::createEditor(const GraphViewTheme &theme) {
  return new KeyboardPanel(this, theme);
}
