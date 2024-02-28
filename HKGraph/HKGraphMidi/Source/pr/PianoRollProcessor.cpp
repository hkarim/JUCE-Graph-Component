#include "PianoRollProcessor.h"

juce::Component* PianoRollProcessor::createEditor(const GraphViewTheme &theme) {
  return new PianoRollProcessor::Panel(this, theme);
}