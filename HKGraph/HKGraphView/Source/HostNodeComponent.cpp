#include "HostNodeComponent.h"
#include "NodeProcessor.h"
#include "ConstrainedComponent.h"

HostNodeComponent::HostNodeComponent(const GraphViewTheme &viewTheme, NodeProcessor *processor)
  : NodeComponent(viewTheme, processor),
    editor(processor->createEditor(viewTheme)),
    resizableComponent(this, &constrains) {
  if (editor) {
    if (auto constrained = dynamic_cast<ConstrainedComponent *>(editor.get())) {
      constrains.setSizeLimits(
        constrained->m_constrains.getMinimumWidth(),
        constrained->m_constrains.getMinimumHeight(),
        constrained->m_constrains.getMaximumWidth(),
        constrained->m_constrains.getMaximumHeight()
      );
    }
  }

}

HostNodeComponent::~HostNodeComponent() = default;


NodeDescriptor *HostNodeComponent::makeDescriptor() {
  auto descriptor = NodeComponent::makeDescriptor();
  descriptor->kind = NodeDescriptorKind::Host;
  return descriptor;
}

HostNodeComponent *HostNodeComponent::fromDescriptor(const NodeDescriptor *descriptor) {
  auto node = new HostNodeComponent(descriptor->theme, descriptor->processor);
  node->apply(descriptor);
  return node;
}

void HostNodeComponent::paint(juce::Graphics &g) {
  juce::Path p;
  auto bounds = boxBounds();
  p.addRoundedRectangle(bounds, 3);

  if (m_processor->is_muted()) {
    if (selected)
      g.setColour(juce::Colours::darkred);
    else
      g.setColour(juce::Colours::black);
  } else {
    if (selected)
      g.setColour(juce::Colour(theme.cNodeBackgroundSelected));
    else
      g.setColour(juce::Colour(cNodeBackgroundCurrent));
  }

  g.fillPath(p);

  g.setColour(juce::Colour(theme.cHostComponentDragAreaForeground));
  g.drawText(
    m_processor->m_name,
    juce::Rectangle<int>(0, 4, bounds.getWidth(), theme.hostComponentDragAreaHeight),
    juce::Justification::centred);
}

void HostNodeComponent::resized() {
  if (editor) {
    auto bounds = boxBounds();
    auto guest = editor.get();
    bounds.removeFromTop(theme.hostComponentDragAreaHeight);
    guest->setBounds(0, theme.hostComponentDragAreaHeight, bounds.getWidth(), bounds.getHeight() + theme.pinHeight);
    addAndMakeVisible(guest);
    ///
    if (dynamic_cast<ConstrainedComponent *>(guest)) {
      addAndMakeVisible(resizableComponent);
      resizableComponent.setSize(10, 10);
      resizableComponent.setAlwaysOnTop(true);
      bounds = getLocalBounds();
      resizableComponent.setTopLeftPosition(bounds.getWidth() - 10, bounds.getHeight() - 20);
//      if (auto parent = getParentComponent()) {
//        parent->childBoundsChanged(this);
//      }
    }
    ///
  }
  NodeComponent::resized();
}
