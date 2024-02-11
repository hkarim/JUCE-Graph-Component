#pragma once

#include "JuceHeader.h"
#include "Graph.h"
#include "NodeComponent.h"

class EdgeComponent : public juce::Component {
public:
  GraphViewTheme theme;
  const NodeComponent::PinComponent *startPin;
  const NodeComponent::PinComponent *endPin;
  Graph::Edge model;
  juce::Path path;
  bool inverted = false;
  bool selected = false;

  EdgeComponent(
    const GraphViewTheme &theme,
    const NodeComponent::PinComponent *startPin,
    const NodeComponent::PinComponent *endPin,
    Graph::Edge graphEdge);

  ~EdgeComponent() override;

  bool hitTest(int x, int y) override;

  void paint(juce::Graphics &g) override;

  bool isConnecting(const NodeComponent::PinComponent *first, const NodeComponent::PinComponent *second) const;

  void debug(const std::string &action) const;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EdgeComponent)
};
