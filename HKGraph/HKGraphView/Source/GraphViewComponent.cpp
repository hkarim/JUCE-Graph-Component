#include "GraphViewComponent.h"
#include "HostNodeComponent.h"
#include "ConstrainedComponent.h"

GraphViewComponent::GraphViewComponent(Graph *sharedGraph) : graph(sharedGraph) {
  selector = std::make_unique<SelectionComponent>(theme.cSelectionBackground);
  edgeDrawer = std::make_unique<UnboundEdgeComponent>();
  mouseListener = std::make_unique<ChildrenMouseListener>(this);
  setWantsKeyboardFocus(true);
}

GraphViewComponent::~GraphViewComponent() {
  for (auto &[_, e]: edges) delete e;
  edges.clear();
  for (auto &[_, n]: nodes) delete n;
  nodes.clear();
}

void GraphViewComponent::paint(juce::Graphics &g) {
  g.fillAll(juce::Colour(theme.cBackground));
}

void GraphViewComponent::resized() {
}

void GraphViewComponent::recordUI(std::unordered_map<uuid, std::unique_ptr<NodeDescriptor>> &nodeDescriptors) {
  for (auto &[_, node]: nodes) {
    auto descriptor = node->makeDescriptor();
    std::unique_ptr<NodeDescriptor> p(descriptor);
    nodeDescriptors[descriptor->processor->m_id] = std::move(p);
  }
}

void GraphViewComponent::restoreUI(std::unordered_map<uuid, std::unique_ptr<NodeDescriptor>> &nodeDescriptors) {
  // erase deleted references
  auto nd_p = std::begin(nodeDescriptors);
  while (nd_p != std::end(nodeDescriptors)) {
    auto existing_p = graph->m_nodes.find(nd_p->first);
    if (existing_p == std::end(graph->m_nodes)) {
      nd_p = nodeDescriptors.erase(nd_p);
    } else {
      ++nd_p;
    }
  }
  //
  for (const auto &[id, ptr]: nodeDescriptors) {
    NodeComponent *node = nullptr;
    if (ptr->kind == NodeDescriptorKind::Plain) {
      node = NodeComponent::fromDescriptor(ptr.get());
    } else if (ptr->kind == NodeDescriptorKind::Host) {
      node = HostNodeComponent::fromDescriptor(ptr.get());
    }

    if (node != nullptr) {
      node->setTransform(ptr->scale.followedBy(ptr->translation));
      node->addMouseListener(mouseListener.get(), true);
      addAndMakeVisible(node);
      nodes[node->m_processor->m_id] = node;
    }
  }

  for (const auto &[id, graphEdge]: graph->m_edges) {
    auto sourceNodePtr = nodes.find(graphEdge.m_source_node_id);
    auto targetNodePtr = nodes.find(graphEdge.m_target_node_id);

    if (sourceNodePtr != std::end(nodes) && targetNodePtr != std::end(nodes)) {
      auto sourcePinPtr = sourceNodePtr->second->outs.find(graphEdge.m_source_pin_id);
      auto targetPinPtr = targetNodePtr->second->ins.find(graphEdge.m_target_pin_id);
      if (sourcePinPtr != std::end(sourceNodePtr->second->outs) &&
          targetPinPtr != std::end(targetNodePtr->second->ins)) {
        auto *edge = new EdgeComponent(theme, sourcePinPtr->second.get(), targetPinPtr->second.get(), graphEdge);
        calculateEdgeBounds(edge);
        edges[edge->model.m_id] = edge;
        edge->addMouseListener(mouseListener.get(), false);
        addAndMakeVisible(edge);
        edge->toBack();
      }
    }
  }

  assertions();
}

void GraphViewComponent::addNode(NodeProcessor *processor, const juce::Point<float> &position) {
  auto ins = processor->m_ins.size();
  auto outs = processor->m_outs.size();
  auto maxNumPins = static_cast<int>(std::max(ins, outs));
  auto w = (maxNumPins * theme.pinWidth) + ((maxNumPins + 1) * theme.pinSpacing);
  addNodeSized(processor, w, theme.nodeHeight, position);
}

void GraphViewComponent::addNodeSized(NodeProcessor *processor,
                                      int width,
                                      int height,
                                      const juce::Point<float> &position) {
  auto ins = processor->m_ins.size();
  auto outs = processor->m_outs.size();
  auto maxNumPins = static_cast<int>(std::max(ins, outs));
  auto w = (maxNumPins * theme.pinWidth) + ((maxNumPins + 1) * theme.pinSpacing);
  w = std::max(w, width);
  auto h = std::max(theme.nodeHeight, height);
  auto node = new NodeComponent(theme, processor);
  node->setBounds(0, 0, w, h);
  node->translation = juce::AffineTransform::translation(position);
  node->scale = juce::AffineTransform::scale(theme.initialScaleFactor);
  node->setTransform(node->translation.followedBy(node->scale));
  node->addMouseListener(mouseListener.get(), true);
  addAndMakeVisible(node);

  nodes[node->m_processor->m_id] = node;
  graph->add_node(processor);
  assertions();
}

void GraphViewComponent::addHostNode(NodeProcessor *processor,
                                     int width,
                                     int height,
                                     const juce::Point<float> &position) {
  auto node = new HostNodeComponent(theme, processor);
  node->setBounds(0, 0, width, height);
  node->translation = juce::AffineTransform::translation(position);
  node->scale = juce::AffineTransform::scale(theme.initialScaleFactor);
  node->setTransform(node->translation.followedBy(node->scale));
  node->addMouseListener(mouseListener.get(), true);
  addAndMakeVisible(node);

  nodes[node->m_processor->m_id] = node;
  graph->add_node(processor);
  assertions();
}

NodeComponent *GraphViewComponent::clone(const NodeComponent *original) {
  auto w = original->getWidth();
  auto h = original->getHeight();
  NodeComponent *node;
  auto processor = original->m_processor->clone();
  if (dynamic_cast<const HostNodeComponent *>(original)) {
    node = new HostNodeComponent(theme, processor);
  } else {
    node = new NodeComponent(theme, processor);
  }

  auto position = juce::Point<float>(original->translation.getTranslationX(), original->translation.getTranslationY());
  position.addXY(20.0f, 20.0f);
  node->setBounds(0, 0, w, h);
  node->translation = juce::AffineTransform::translation(position);
  node->scale = original->scale;
  node->scaleFactor = original->scaleFactor;
  node->setTransform(node->scale.followedBy(node->translation));
  node->addMouseListener(mouseListener.get(), true);
  addAndMakeVisible(node);

  nodes[node->m_processor->m_id] = node;
  graph->add_node(processor);
  assertions();
  return node;
}

std::optional<EdgeComponent *>
GraphViewComponent::addEdge(NodeComponent::PinComponent *source, NodeComponent::PinComponent *target) {
  std::optional<EdgeComponent *> result = std::nullopt;
  auto opt = graph->connect(source->model.m_id, target->model.m_id);
  if (opt.has_value()) {
    auto model = opt.value();
    auto *edge = new EdgeComponent(theme, source, target, model);
    calculateEdgeBounds(edge);
    edges[edge->model.m_id] = edge;
    edge->addMouseListener(mouseListener.get(), false);
    addAndMakeVisible(edge);
    edge->toBack();
    result = edge;
  }
  assertions();
  return result;
}

void GraphViewComponent::assertions() const {
  jassert(graph->m_edges.size() == edges.size());
  jassert(graph->m_nodes.size() == nodes.size());
}

void GraphViewComponent::pinMouseDrag(NodeComponent::PinComponent *pin, const juce::MouseEvent &e) {
  auto relativeEvent = e.getEventRelativeTo(this);
  auto position = relativeEvent.getPosition();
  edgeDrawer->startPin = pin;
  edgeDrawer->endPin = nullptr;
  edgeDrawer->currentEndPosition = position;
  edgeDrawer->calculateBounds(getLocalPoint(pin, juce::Point<int>(theme.pinWidth / 2, theme.pinHeight / 2)), position);
  addAndMakeVisible(edgeDrawer.get());
}

void GraphViewComponent::pinMouseUp(NodeComponent::PinComponent *pin, const juce::MouseEvent &e) {
  juce::ignoreUnused(pin);
  auto relativeEvent = e.getEventRelativeTo(this);
  auto position = relativeEvent.getPosition();
  edgeDrawer->currentEndPosition = position;
  removeChildComponent(edgeDrawer.get());
}

void GraphViewComponent::pinMouseEnter(NodeComponent::PinComponent *pin, const juce::MouseEvent &e) {
  auto relativeEvent = e.getEventRelativeTo(this);
  auto position = relativeEvent.getPosition();
  if (edgeDrawer->currentEndPosition == position) {
    drawConnector(pin);
  }
}

void GraphViewComponent::drawConnector(NodeComponent::PinComponent *pin) {
  if (edgeDrawer->startPin && edgeDrawer->startPin != pin) {
    edgeDrawer->endPin = pin;
    if (isLegalEdge(edgeDrawer->startPin, edgeDrawer->endPin)) {
      if (edgeDrawer->startPin->model.m_kind == Graph::Node::PinKind::Out &&
          edgeDrawer->endPin->model.m_kind == Graph::Node::PinKind::In) {
        addEdge(edgeDrawer->startPin, edgeDrawer->endPin);
      } else {
        addEdge(edgeDrawer->endPin, edgeDrawer->startPin);
      }
    }
  }
}

bool GraphViewComponent::isLegalEdge(NodeComponent::PinComponent *start, NodeComponent::PinComponent *end) {
  auto existing =
    find_if(
      std::begin(edges),
      std::end(edges),
      [&](auto &e) -> bool {
        return e.second->isConnecting(start, end);
      });
  return
    existing == std::end(edges) &&
    start->model.m_kind != end->model.m_kind &&
    start->owner != end->owner;
}

void GraphViewComponent::nodeMouseDrag(NodeComponent *node, const juce::MouseEvent &mouseEvent) {
  node->selected = true;
  node->repaint();
  if (!mouseEvent.mods.isShiftDown() && !nodeMultiSelectionOn) {
    for (auto &[_, n]: nodes) {
      if (n != node) {
        n->selected = false;
        n->repaint();
      }
    }
  }
  auto offset = mouseEvent.getOffsetFromDragStart();
  node->setTransform(node->scale.followedBy(node->translation.translated(offset * node->scaleFactor)));
  for (auto &[_, n]: nodes) {
    if (n == node || !(n->selected)) continue;
    // using node->scaleFactor here is not a mistake, we need to move according to the dragged node scale
    n->setTransform(n->scale.followedBy(n->translation.translated(offset * node->scaleFactor)));
  }
  for (auto &[_, e]: edges) {
    calculateEdgeBounds(e);
    e->repaint();
  }
}

void GraphViewComponent::nodeMouseDown(NodeComponent *node, const juce::MouseEvent &mouseEvent) {
  recordState();
  if (!mouseEvent.mods.isShiftDown() && nodeMultiSelectionOn && !node->selected) {
    nodeMultiSelectionOn = false;
  }
  node->selected = !node->selected;
  node->repaint();

  if (!mouseEvent.mods.isShiftDown() && !nodeMultiSelectionOn) {
    for (auto &[_, n]: nodes) {
      if (n != node) {
        n->selected = false;
        n->repaint();
      }
    }
    for (auto &[_, e]: edges) {
      e->selected = false;
      e->repaint();
    }
  } else {
    nodeMultiSelectionOn = true;
  }
}

void GraphViewComponent::nodeMouseUp(NodeComponent *node, const juce::MouseEvent &mouseEvent) {
  juce::ignoreUnused(node, mouseEvent);
  recordState();
}

void GraphViewComponent::nodeMouseDoubleClick(NodeComponent *node, const juce::MouseEvent &) {
  if (auto *n = dynamic_cast<HostNodeComponent*>(node)) {
    n->toggleSize();
    childBoundsChanged(node);
  }
}

void GraphViewComponent::edgeMouseDown(EdgeComponent *edge, const juce::MouseEvent &mouseEvent) {

  if (!mouseEvent.mods.isShiftDown() && edgeMultiSelectionOn && !edge->selected) {
    edgeMultiSelectionOn = false;
  }

  edge->selected = !edge->selected;
  edge->repaint();

  if (!mouseEvent.mods.isShiftDown() && !edgeMultiSelectionOn) {
    for (auto &[_, e]: edges) {
      if (e != edge) {
        e->selected = false;
        e->repaint();
      }
    }
    for (auto &[_, n]: nodes) {
      n->selected = false;
      n->repaint();
    }
  } else {
    edgeMultiSelectionOn = true;
  }
}

void GraphViewComponent::edgeMouseDrag(EdgeComponent *edge, const juce::MouseEvent &e) {
  auto relativeEvent = e.getEventRelativeTo(this);
  auto position = relativeEvent.getPosition();
  if (!edge->dragging) {
    edge->dragging = true;
    edge->distanceFromStart = position.getDistanceFrom(getLocalPoint(edge->startPin, edge->startPin->getPosition()));
    edge->distanceFromEnd = position.getDistanceFrom(getLocalPoint(edge->endPin, edge->endPin->getPosition()));
    edgeDrawer->currentEndPosition = position;
    removeChildComponent(edge);
  }
  if (edge->distanceFromStart <= edge->distanceFromEnd) {
    pinMouseDrag(edge->endPin, relativeEvent);
  } else {
    pinMouseDrag(edge->startPin, relativeEvent);
  }
}

void GraphViewComponent::edgeMouseUp(EdgeComponent *edge, const juce::MouseEvent &e) {
  if (edge->dragging) {
    removeChildComponent(edgeDrawer.get());
    // simulate a double click, which will remove the edge along with the graph edge model
    edgeMouseDoubleClick(edge, e);
  }
  assertions();
}

void GraphViewComponent::calculateEdgeBounds(EdgeComponent *edge) {

  auto startPin = edge->startPin;
  auto endPin = edge->endPin;

  auto startPinTopLeft = getLocalPoint(startPin, juce::Point<int>(0, 0));
  auto endPinTopLeft = getLocalPoint(endPin, juce::Point<int>(0, 0));

  auto startPinBottomRight = getLocalPoint(startPin, juce::Point<int>(startPin->getWidth(), startPin->getHeight()));
  auto endPinBottomRight = getLocalPoint(endPin, juce::Point<int>(endPin->getWidth(), endPin->getHeight()));

  auto left = std::min(startPinTopLeft.x, endPinTopLeft.x);
  auto right = std::max(startPinBottomRight.x, endPinBottomRight.x);
  auto top = std::min(startPinTopLeft.y, endPinTopLeft.y);
  auto bottom = std::max(startPinBottomRight.y, endPinBottomRight.y);

  auto w = right - left;
  auto h = bottom - top;

  edge->inverted = left != startPinTopLeft.x;

  edge->setBounds(left, top, w, h);
}

void GraphViewComponent::edgeMouseDoubleClick(EdgeComponent *edge, const juce::MouseEvent &e) {
  juce::ignoreUnused(e);
  if (graph->disconnect(edge->model.m_id)) {
    edge->removeMouseListener(mouseListener.get());
    removeChildComponent(edge);
    edges.erase(edge->model.m_id);
    delete edge;
  }
  assertions();
}

void GraphViewComponent::recordState() {
  for (auto &[_, n]: nodes) {
    auto p = getLocalPoint(n, juce::Point<int>(0, 0));
    n->translation = juce::AffineTransform::translation(p);
    n->scale = juce::AffineTransform::scale(n->scaleFactor);
  }
}

void GraphViewComponent::mouseDown(const juce::MouseEvent &mouseEvent) {

  if (mouseEvent.mods.isPopupMenu()) {
    popupMenu(mouseEvent);
    return;
  }

  // node selection
  for (auto &[_, n]: nodes) {
    n->selected = false;
    n->repaint();
  }
  nodeMultiSelectionOn = false;

  // edge selection
  for (auto &[_, e]: edges) {
    e->selected = false;
    e->repaint();
  }
  edgeMultiSelectionOn = false;

  // selector
  auto position = mouseEvent.getMouseDownPosition();
  selector->setBounds(position.x, position.y, 10, 10);
  addAndMakeVisible(selector.get());
}

void GraphViewComponent::mouseDrag(const juce::MouseEvent &mouseEvent) {

  // draw a selection box
  auto position = mouseEvent.getMouseDownPosition();
  auto offset = mouseEvent.getOffsetFromDragStart();


  selector->calculateBounds(position, offset);

  // handle node selection
  auto numSelectedNodes = 0;
  for (auto &[_, n]: nodes) {
    auto selected = selector->getBounds().intersects(n->getBounds().transformedBy(n->getTransform()));
    n->selected = selected;
    n->repaint();
    if (selected) ++numSelectedNodes;
  }

  nodeMultiSelectionOn = numSelectedNodes > 0;


  // handle edge selection
  auto numSelectedEdges = 0;
  for (auto &[_, e]: edges) {
    auto intersection = selector->getBounds().getIntersection(e->getBounds());
    auto selected = intersection == e->getBounds() || intersection.getWidth() >= e->getBounds().getWidth();
    e->selected = selected;
    e->repaint();
    if (selected) ++numSelectedEdges;
  }

  edgeMultiSelectionOn = numSelectedEdges > 0;

}

void GraphViewComponent::mouseUp(const juce::MouseEvent &mouseEvent) {
  juce::ignoreUnused(mouseEvent);
  removeChildComponent(selector.get());
}

void GraphViewComponent::childBoundsChanged(juce::Component *) {
  for (auto &[_, e]: edges) {
    calculateEdgeBounds(e);
    e->repaint();
  }
}

bool GraphViewComponent::keyPressed(const juce::KeyPress &key) {

  auto code = key.getKeyCode();
  auto commandDown = key.getModifiers().isCommandDown();

  if (code == juce::KeyPress::deleteKey || code == juce::KeyPress::backspaceKey) {
    removeSelected();
  }
    // cmd '0'
  else if (code == 48 && commandDown) {
    zoomToOriginalSize();
  }
    // cmd '+' or cmd '='
  else if ((code == 43 || code == 61) && commandDown) {
    zoomIn();
  }
    // cmd '-'
  else if (code == 45 && commandDown) {
    zoomOut();
  }
    // cmd 'a'
  else if (code == 65 && commandDown) {
    selectAll();
  }
    // cmd 'd'
  else if (code == 68 && commandDown) {
    duplicate();
  }
    // cmd 'm'
  else if (code == 77 && commandDown) {
    mute();
  }


  return true;
}

void GraphViewComponent::zoomIn() {

  recordState();

  for (auto &[_, n]: nodes) {
    if (n->selected && n->scaleFactor <= theme.scaleMax) {
      n->scaleFactor += theme.scaleStep;
      n->scale = juce::AffineTransform::scale(n->scaleFactor);
      n->setTransform(n->scale.followedBy(n->translation));
    }
  }

  for (auto &[_, e]: edges) {
    calculateEdgeBounds(e);
    e->repaint();
  }
}

void GraphViewComponent::zoomOut() {

  recordState();

  for (auto &[_, n]: nodes) {
    if (n->selected && n->scaleFactor >= theme.scaleMin) {
      n->scaleFactor -= theme.scaleStep;
      n->scale = juce::AffineTransform::scale(n->scaleFactor);
      n->setTransform(n->scale.followedBy(n->translation));
    }

  }

  for (auto &[_, e]: edges) {
    calculateEdgeBounds(e);
    e->repaint();
  }
}

void GraphViewComponent::zoomToOriginalSize() {

  recordState();

  for (auto &[_, n]: nodes) {
    if (n->selected) {
      n->scaleFactor = theme.initialScaleFactor;
      n->scale = juce::AffineTransform::scale(n->scaleFactor);
      n->setTransform(n->scale.followedBy(n->translation));
    }
  }

  for (auto &[_, e]: edges) {
    calculateEdgeBounds(e);
    e->repaint();
  }
}

void GraphViewComponent::removeSelected() {
  // remove edges
  auto edge_p = std::begin(edges);
  while (edge_p != std::end(edges)) {
    EdgeComponent *e = edge_p->second;
    if (e->selected) {
      if (graph->disconnect(edge_p->first)) {
        e->removeMouseListener(mouseListener.get());
        removeChildComponent(e);
        edge_p = edges.erase(edge_p);
        delete e;
      }
    } else {
      ++edge_p;
    }
  }

  assertions();
  edgeMultiSelectionOn = false;

  // remove the nodes
  auto node_p = std::begin(nodes);
  while (node_p != std::end(nodes)) {
    NodeComponent *n = node_p->second;
    if (n->selected && !n->m_processor->m_sticky) {
      // delete connected edges
      edge_p = std::begin(edges);
      while (edge_p != std::end(edges)) {
        EdgeComponent *e = edge_p->second;
        if (e->model.m_source_node_id == n->m_processor->m_id || e->model.m_target_node_id == n->m_processor->m_id) {
          if (graph->disconnect(e->model.m_id)) {
            e->removeMouseListener(mouseListener.get());
            removeChildComponent(e);
            edge_p = edges.erase(edge_p);
            delete e;
          }
        } else {
          ++edge_p;
        }
      }
      // delete the node
      graph->remove_node(n->m_processor->m_id);
      node_p = nodes.erase(node_p);
      n->removeMouseListener(mouseListener.get());
      removeChildComponent(n);
      delete n;
    } else {
      ++node_p;
    }
  }
  assertions();
  nodeMultiSelectionOn = false;
}

void GraphViewComponent::selectAll() {

  for (auto &[_, n]: nodes) {
    n->selected = true;
  }
  nodeMultiSelectionOn = true;

  for (auto &[_, e]: edges) {
    e->selected = true;
  }
  edgeMultiSelectionOn = true;

  repaint();
}

void GraphViewComponent::duplicate() {
  std::vector<NodeComponent *> selectedNodes;
  for (auto &[_, n]: nodes) {
    if (n->selected && !n->m_processor->m_sticky) {
      selectedNodes.push_back(n);
    }
  }

  std::map<int, NodeComponent *> clonedNodes;
  auto i = 0;
  for (auto &n: selectedNodes) {
    n->selected = false;
    auto c = clone(n);
    c->selected = true;
    clonedNodes[i++] = c;
  }

  std::vector<std::pair<NodeComponent::PinComponent *, NodeComponent::PinComponent *>> connections;
  auto s = 0;
  for (auto &source: selectedNodes) {
    auto t = 0;
    for (auto &target: selectedNodes) {
      for (auto &[_, e]: edges) {
        if (e->startPin->owner == source && e->endPin->owner == target) {
          auto startPinOrder = static_cast<unsigned long>(e->startPin->model.m_order);
          auto endPinOrder = static_cast<unsigned long>(e->endPin->model.m_order);
          auto clonedSource = clonedNodes[s];
          auto clonedTarget = clonedNodes[t];
          auto clonedSourcePinPtr =
            std::find_if(clonedSource->outs.begin(), clonedSource->outs.end(), [&](const auto &x) {
              return x.second->model.m_order == startPinOrder;
            });
          auto clonedTargetPinPtr =
            std::find_if(clonedTarget->ins.begin(), clonedTarget->ins.end(), [&](const auto &x) {
              return x.second->model.m_order == endPinOrder;
            });
          if (clonedSourcePinPtr != std::end(clonedSource->outs) &&
              clonedTargetPinPtr != std::end(clonedTarget->ins)) {
            connections.emplace_back(clonedSourcePinPtr->second.get(), clonedTargetPinPtr->second.get());
            e->selected = false;
          }
        }
      }
      t++;
    }
    s++;
  }

  for (auto &t: connections) {
    auto result = addEdge(t.first, t.second);
    if (result != std::nullopt) {
      result.value()->selected = true;
    }
  }

  repaint();
}

void GraphViewComponent::mute() {
  for (auto &[_, n]: nodes) {
    if (n->selected && !n->m_processor->m_sticky) {
      n->m_processor->toggle_muted(graph);
    }
  }
  repaint();
}

void GraphViewComponent::debug(const std::string &action) const {
  std::cout << "[GraphViewComponent::" << action << "]"
            << " node-size = " << nodes.size()
            << ", edge-size = " << edges.size()
            << std::endl;
}
