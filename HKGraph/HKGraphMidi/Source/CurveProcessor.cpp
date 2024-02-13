#include "CurveProcessor.h"
#include "CurvePanel.h"

juce::Component *CurveProcessor::createEditor(const GraphViewTheme &theme) {
  return new CurvePanel(this, theme);
}
