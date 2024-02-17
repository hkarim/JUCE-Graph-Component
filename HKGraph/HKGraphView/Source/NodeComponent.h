#pragma once

#include "JuceHeader.h"
#include "Graph.h"
#include "GraphViewTheme.h"
#include "NodeDescriptor.h"
#include "NodeProcessor.h"

class NodeComponent : public juce::Component {
public:
class PinComponent : public juce::Component, public juce::SettableTooltipClient {
  public:
    PinComponent(const NodeComponent *node, const Graph::Node::Pin &pin);

    void paint(juce::Graphics &g) override;

  public:
    const NodeComponent *owner;
    const Graph::Node::Pin model;
    unsigned int cPinBackgroundCurrent;
  private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PinComponent)
  };

  NodeComponent(const GraphViewTheme &graphViewTheme, NodeProcessor *processor);

  virtual NodeDescriptor *makeDescriptor() const;

  static NodeComponent *fromDescriptor(const NodeDescriptor *descriptor);

  void apply(const NodeDescriptor *descriptor);

  ~NodeComponent() override;

  [[nodiscard]] bool hasIns() const;

  [[nodiscard]] bool hasOuts() const;

  int boxHeight();

  juce::Rectangle<int> boxBounds();

  void paint(juce::Graphics &g) override;

  void resized() override;

  bool hitTest(int x, int y) override;

  void mouseDown(const juce::MouseEvent &e) override;

  void mouseDrag(const juce::MouseEvent &e) override;

  void mouseUp(const juce::MouseEvent &e) override;

  void mouseMove(const juce::MouseEvent &e) override;

  void mouseDoubleClick(const juce::MouseEvent &e) override;

  void mouseExit(const juce::MouseEvent &e) override;

  virtual NodeComponent *clone(const GraphViewTheme &graphViewTheme, NodeProcessor *processor) const;

  void debug(const std::string &action) const;

public:
  const GraphViewTheme theme;
  NodeProcessor *m_processor;
  std::unordered_map<uuid, std::unique_ptr<PinComponent>> ins;
  std::vector<PinComponent*> ordered_ins;
  std::unordered_map<uuid, std::unique_ptr<PinComponent>> outs;
  std::vector<PinComponent*> ordered_outs;
  bool selected{};
  unsigned int cNodeBackgroundCurrent;
  float scaleFactor;
  juce::AffineTransform translation;
  juce::AffineTransform scale;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NodeComponent)
};

