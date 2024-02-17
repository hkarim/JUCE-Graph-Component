#pragma once

#include "NodeComponent.h"
#include "NodeProcessor.h"

class HostNodeComponent : public NodeComponent {

public:
  std::unique_ptr<Component> editor;
  juce::ComponentBoundsConstrainer constrains;
  juce::ResizableCornerComponent resizableComponent;


  HostNodeComponent(const GraphViewTheme& theme, NodeProcessor *processor);

  ~HostNodeComponent() override;

  NodeDescriptor* makeDescriptor() override;

  static HostNodeComponent* fromDescriptor(const NodeDescriptor *descriptor);

  void paint(juce::Graphics& g) override;

  void resized() override;

  void toggleSize();

  void minimize();

  void maximize();

private:
  int expandedWidth{};
  int expandedHeight{};
  bool expanded{true};
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HostNodeComponent)

};


