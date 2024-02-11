#include "NodeComponent.h"

NodeComponent::NodeComponent(
  const GraphViewTheme &viewTheme,
  NodeProcessor *processor)
  : theme(viewTheme),
    m_processor(processor) {
  cNodeBackgroundCurrent = theme.cNodeBackground;
  for (auto &in: m_processor->m_ins) {
    auto pin = std::make_unique<PinComponent>(this, in.second);
    pin->setTooltip(juce::String(pin->model.m_order + 1));
    ordered_ins.push_back(pin.get());
    ins[pin->model.m_id] = std::move(pin);
  }
  std::sort(std::begin(ordered_ins), std::end(ordered_ins),
            [&](auto &l, auto &r) { return l->model.m_order < r->model.m_order; });

  for (auto &out: m_processor->m_outs) {
    auto pin = std::make_unique<PinComponent>(this, out.second);
    pin->setTooltip(juce::String(pin->model.m_order + 1));
    ordered_outs.push_back(pin.get());
    outs[pin->model.m_id] = std::move(pin);
  }
  std::sort(std::begin(ordered_outs), std::end(ordered_outs),
            [&](auto &l, auto &r) { return l->model.m_order < r->model.m_order; });

  translation = juce::AffineTransform::translation(0, 0);
  scaleFactor = theme.initialScaleFactor;
  scale = juce::AffineTransform::scale(scaleFactor);
}

NodeComponent::~NodeComponent() = default;


void NodeComponent::apply(const NodeDescriptor *descriptor) {
  this->selected = descriptor->selected;
  this->cNodeBackgroundCurrent = descriptor->cNodeBackgroundCurrent;
  this->scaleFactor = descriptor->scaleFactor;
  this->translation = descriptor->translation;
  this->scale = descriptor->scale;
  this->setBounds(
    static_cast<int>(descriptor->position.x),
    static_cast<int>(descriptor->position.y),
    descriptor->width,
    descriptor->height);
}

NodeDescriptor *NodeComponent::makeDescriptor() {
  auto descriptor = new NodeDescriptor;
  descriptor->kind = NodeDescriptorKind::Plain;
  descriptor->processor = this->m_processor;
  descriptor->theme = this->theme;
  descriptor->selected = this->selected;
  descriptor->cNodeBackgroundCurrent = this->cNodeBackgroundCurrent;
  auto position = this->getPosition().toFloat();
  descriptor->position.setXY(position.x, position.y);
  descriptor->width = this->getWidth();
  descriptor->height = this->getHeight();
  descriptor->scaleFactor = this->scaleFactor;
  descriptor->translation = this->translation;
  descriptor->scale = this->scale;
  return descriptor;
}

NodeComponent *NodeComponent::fromDescriptor(const NodeDescriptor *descriptor) {
  auto node = new NodeComponent(descriptor->theme, descriptor->processor);
  node->apply(descriptor);
  return node;
}

bool NodeComponent::hasIns() const {
  return !ins.empty();
}

bool NodeComponent::hasOuts() const {
  return !outs.empty();
}

int NodeComponent::boxHeight() {
  auto h = getHeight();
  if (hasIns()) h -= theme.pinHeight;
  if (hasOuts()) h -= theme.pinHeight;
  return h;
}

juce::Rectangle<int> NodeComponent::boxBounds() {
  auto local = getLocalBounds();
  if (hasIns()) local.removeFromTop(theme.pinHeight);
  if (hasOuts()) local.removeFromBottom(theme.pinHeight);
  return local;
}

void NodeComponent::paint(juce::Graphics &g) {
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
  g.setColour(juce::Colours::white);
  g.drawText(m_processor->m_name, boxBounds(), juce::Justification::centred, true);
}

void NodeComponent::resized() {
  {
    auto x = theme.pinSpacing;
    auto y = 0;
    auto w = theme.pinWidth;
    auto h = theme.pinHeight;
    for (auto &p: ordered_ins) {
      p->setBounds(x, y, w, h);
      addAndMakeVisible(p);
      x += w + theme.pinSpacing;
    }
  }
  {
    auto x = theme.pinSpacing;
    auto y = getHeight() - theme.pinHeight;
    auto w = theme.pinWidth;
    auto h = theme.pinHeight;
    for (auto &p: ordered_outs) {
      p->setBounds(x, y, w, h);
      addAndMakeVisible(p);
      x += w + theme.pinSpacing;
    }
  }
}

bool NodeComponent::hitTest(int x, int y) {
  auto inBox = boxBounds().contains(x, y);
  auto inTopPins = false;
  auto inBottomPins = false;

  for (auto &[_, p]: ins) {
    if (p->getBounds().contains(x, y)) {
      inTopPins = true;
      break;
    }
  }

  for (auto &[_, p]: outs) {
    if (p->getBounds().contains(x, y)) {
      inBottomPins = true;
      break;
    }
  }

  return inBox || inTopPins || inBottomPins;
}

void NodeComponent::mouseDown(const juce::MouseEvent &mouseEvent) {
  juce::ignoreUnused(mouseEvent);
}

void NodeComponent::mouseDrag(const juce::MouseEvent &mouseEvent) {
  juce::ignoreUnused(mouseEvent);
}

void NodeComponent::mouseUp(const juce::MouseEvent &mouseEvent) {
  juce::ignoreUnused(mouseEvent);
}

void NodeComponent::mouseMove(const juce::MouseEvent &mouseEvent) {
  if (boxBounds().contains(mouseEvent.x, mouseEvent.y)) {
    cNodeBackgroundCurrent = theme.cNodeBackgroundHover;
    repaint();
  }
}

void NodeComponent::mouseExit(const juce::MouseEvent &mouseEvent) {
  juce::ignoreUnused(mouseEvent);
  cNodeBackgroundCurrent = theme.cNodeBackground;
  repaint();
}

void NodeComponent::mouseDoubleClick(const juce::MouseEvent &mouseEvent) {
  juce::ignoreUnused(mouseEvent);
}

void NodeComponent::debug(const std::string &action) const {
  std::cout
    << "[NodeComponent::" << action << "]"
    << " id = " << m_processor->m_id
    << ", name = " << m_processor->m_name
    << ", ins = " << ins.size()
    << ", outs = " << outs.size()
    << std::endl;
}

NodeComponent::PinComponent::PinComponent(
  const NodeComponent *node,
  const Graph::Node::Pin &pin)
  : owner(node),
    model(pin) {
  cPinBackgroundCurrent = node->theme.cPinBackground;
}

void NodeComponent::PinComponent::paint(juce::Graphics &g) {
  juce::Path p;
  p.addRectangle(0, 0, static_cast<float>(owner->theme.pinWidth), static_cast<float>(owner->theme.pinHeight));
  g.setColour(juce::Colour(cPinBackgroundCurrent));
  g.fillPath(p);
}



