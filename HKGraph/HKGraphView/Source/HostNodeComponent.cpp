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

  if (editor) {
    auto guest = editor.get();
    addAndMakeVisible(guest);
    if (dynamic_cast<ConstrainedComponent *>(guest)) {
      addAndMakeVisible(resizableComponent);
    }
  }
}

HostNodeComponent::~HostNodeComponent() = default;

NodeDescriptor *HostNodeComponent::makeDescriptor() const {
  auto descriptor = NodeComponent::makeDescriptor();
  descriptor->kind = NodeDescriptorKind::Host;
  descriptor->nodeData.setProperty("expanded", juce::var(expanded), nullptr);
  descriptor->nodeData.setProperty("expandedWidth", juce::var(expandedWidth), nullptr);
  descriptor->nodeData.setProperty("expandedHeight", juce::var(expandedHeight), nullptr);
  return descriptor;
}

HostNodeComponent *HostNodeComponent::fromDescriptor(const NodeDescriptor *descriptor) {
  auto node = new HostNodeComponent(descriptor->theme, descriptor->processor);
  node->apply(descriptor);
  node->expanded = descriptor->nodeData.getProperty("expanded");
  node->expandedWidth = descriptor->nodeData.getProperty("expandedWidth");
  node->expandedHeight = descriptor->nodeData.getProperty("expandedHeight");
  node->setSize(node->expandedWidth, node->expandedHeight);
  if (node->expanded) {
    node->maximize();
  } else {
    node->minimize();
  }
  return node;
}

void HostNodeComponent::paint(juce::Graphics &g) {
  if (expanded) {
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
  } else {
    auto ins = m_processor->m_ins.size();
    auto outs = m_processor->m_outs.size();
    auto maxNumPins = static_cast<int>(std::max(ins, outs));
    auto w = (maxNumPins * theme.pinWidth) + ((maxNumPins + 1) * theme.pinSpacing);
    auto h = theme.nodeHeight;
    setSize(w, h);
    NodeComponent::paint(g);
  }

}

void HostNodeComponent::resized() {
  if (expanded) {
    if (editor) {
      auto bounds = boxBounds();
      bounds.removeFromTop(theme.hostComponentDragAreaHeight);
      auto guest = editor.get();
      guest->setBounds(0, theme.hostComponentDragAreaHeight, bounds.getWidth(), bounds.getHeight() + theme.pinHeight);
      if (dynamic_cast<ConstrainedComponent *>(guest)) {
        resizableComponent.setSize(10, 10);
        resizableComponent.setAlwaysOnTop(true);
        bounds = getLocalBounds();
        resizableComponent.setTopLeftPosition(bounds.getWidth() - 10, bounds.getHeight() - 20);
      }
    }
  }
  NodeComponent::resized();
}

void HostNodeComponent::toggleSize() {
  if (expanded)
    minimize();
  else
    maximize();
}

void HostNodeComponent::minimize() {
  expandedWidth = getWidth();
  expandedHeight = getHeight();
  expanded = false;
  if (editor) {
    removeChildComponent(editor.get());
    removeChildComponent(&resizableComponent);
  }
}

void HostNodeComponent::maximize() {
  expanded = true;
  if (editor) {
    auto guest = editor.get();
    addAndMakeVisible(guest);
    if (dynamic_cast<ConstrainedComponent *>(guest)) {
      addAndMakeVisible(resizableComponent);
    }
  }
  // must set size after adding the components, or the components won't show up
  setSize(expandedWidth, expandedHeight);
}

NodeComponent *HostNodeComponent::clone(const GraphViewTheme &graphViewTheme, NodeProcessor *processor) const {
  auto node = new HostNodeComponent(graphViewTheme, processor);
  node->expanded = this->expanded;
  node->expandedWidth = this->expandedWidth;
  node->expandedHeight = this->expandedHeight;
  if (node->expanded) {
    node->maximize();
  } else {
    if (editor) {
      node->removeChildComponent(node->editor.get());
      node->removeChildComponent(&node->resizableComponent);
    }
  }
  return node;
}
