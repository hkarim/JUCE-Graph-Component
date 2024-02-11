#pragma once

#include "JuceHeader.h"
#include "NodeComponent.h"

class UnboundEdgeComponent : public juce::Component {
public:
  bool inverted = false;
  NodeComponent::PinComponent *startPin;
  NodeComponent::PinComponent *endPin;
  juce::Point<int> currentEndPosition;

  UnboundEdgeComponent();

  void paint(juce::Graphics &g) override;

  void calculateBounds(const juce::Point<int> &start, const juce::Point<int> &end);

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UnboundEdgeComponent)
};
