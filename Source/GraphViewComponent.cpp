
#include "GraphViewComponent.h"
#include "SampleHostedComponents.h"

GraphViewComponent::GraphViewComponent() {
    graph = std::make_unique<Graph>();
    
    selector = std::make_unique<SelectionComponent>(theme.cSelectionBackground);
    edgeDrawer = std::make_unique<UnboundEdgeComponent>();
    
    mouseListener = std::make_unique<ChildrenMouseListener>(this);
    
    setWantsKeyboardFocus(true);
    
}

GraphViewComponent::~GraphViewComponent() {
}

void GraphViewComponent::paint(Graphics& g) {
    g.fillAll(Colour(theme.cBackground));
}

void GraphViewComponent::resized() {
}


NodeComponent* GraphViewComponent::addNode(const std::string& name,
                                           const int ins,
                                           const int outs,
                                           const Point<float> position) {
    auto maxNumPins = std::max(ins, outs);
    auto w = (maxNumPins * theme.pinWidth) + ( (maxNumPins + 1) * theme.pinSpacing);
    
    auto model = graph->addNode(name, ins, outs);
    
    auto node = std::make_unique<NodeComponent>(theme, model);
    node->setBounds(0, 0, w, theme.nodeHeight);
    node->translation = AffineTransform::translation(position);
    node->scale = AffineTransform::scale(theme.initialScaleFactor);
    node->setTransform(node->translation.followedBy(node->scale));
    node->addMouseListener(mouseListener.get(), true);
    addAndMakeVisible(node.get());
    
    auto ptr = node.get();
    nodes.push_back(std::move(node));
    
    assertions();
    
    return ptr;
}

HostNodeComponent* GraphViewComponent::addHostNode(GraphNodeEditor* editor,
                                                   const int ins,
                                                   const int outs,
                                                   const int width,
                                                   const int height,
                                                   const Point<float> position) {
    const auto model = graph->addNode("GraphNodeEditor", ins, outs);
    auto node = std::make_unique<HostNodeComponent>(theme, model, editor);
    node->setBounds(0, 0, width, height);
    node->translation = AffineTransform::translation(position);
    node->scale = AffineTransform::scale(theme.initialScaleFactor);
    node->setTransform(node->translation.followedBy(node->scale));
    node->addMouseListener(mouseListener.get(), true);
    addAndMakeVisible(node.get());
    
    auto ptr = node.get();
    nodes.push_back(std::move(node));
    
    assertions();
    
    return ptr;
}


void GraphViewComponent::removeNode(NodeComponent* n) {
    n->removeMouseListener(mouseListener.get());
    // we need to do this instead of directly erasing because we have to remove the corresponding UI component as well
    std::vector<EdgeComponent*> edgesToDelete;
    for (auto& e : edges) {
        if (e.get()) {
            if ( (e->startPin && e->startPin->node == n) || (e->endPin && e->endPin->node == n)) {
                edgesToDelete.push_back(e.get());
            }
        }
    }
    for (auto& e : edgesToDelete) {
        removeEdge(e);
    }
    
    
    auto ref = std::find_if(std::begin(nodes), std::end(nodes), [&](auto& current) -> bool { return current.get() == n; });
    
    if (ref != nodes.end()) {
        auto n = (*ref).get();
        graph->removeNode(n->model);
        removeChildComponent(n);
        nodes.erase(ref);
    }
    
    assertions();
    
}

EdgeComponent* GraphViewComponent::addEdge(NodeComponent::PinComponent* source, NodeComponent::PinComponent* target) {
    const auto model = graph->addEdge(source->model, target->model);
    
    auto edge = std::make_unique<EdgeComponent>(theme, source, target, model);
    
    calculateEdgeBounds(edge.get());
    edge->addMouseListener(mouseListener.get(), false);
    addAndMakeVisible(edge.get());
    edge->toBack();
    auto ptr = edge.get();
    edges.push_back(std::move(edge));
    
    assertions();
    
    return ptr;
}

void GraphViewComponent::removeEdge(EdgeComponent* e) {
    e->removeMouseListener(mouseListener.get());
    auto ref = std::find_if(std::begin(edges), std::end(edges), [&](auto& current) -> bool { return current.get() == e; });
    
    if (ref != edges.end()) {
        auto e = (*ref).get();
        graph->removeEdge(e->model);
        removeChildComponent(e);
        edges.erase(ref);
    }
    
    assertions();
}

void GraphViewComponent::assertions() const {
    jassert(graph->edges.size() == edges.size());
    jassert(graph->nodes.size() == nodes.size());
}



void GraphViewComponent::pinMouseDrag(NodeComponent::PinComponent* pin, const MouseEvent& e) {
    
    auto relativeEvent = e.getEventRelativeTo(this);
    auto position = relativeEvent.getPosition();
    
    
    edgeDrawer->startPin = pin;
    
    edgeDrawer->endPin = nullptr;
    
    edgeDrawer->currentEndPosition = position;
    
    edgeDrawer->calculateBounds(getLocalPoint(pin, Point<int>(theme.pinWidth/2, theme.pinHeight/2)), position);
    
    addAndMakeVisible(edgeDrawer.get());
}

void GraphViewComponent::pinMouseUp(NodeComponent::PinComponent* pin, const MouseEvent& e) {
    
    auto relativeEvent = e.getEventRelativeTo(this);
    auto position = relativeEvent.getPosition();
    
    edgeDrawer->currentEndPosition = position;
    
    removeChildComponent(edgeDrawer.get());
    
}

void GraphViewComponent::pinMouseEnter(NodeComponent::PinComponent* pin, const MouseEvent& e) {
    
    auto relativeEvent = e.getEventRelativeTo(this);
    auto position = relativeEvent.getPosition();
    
    
    if (edgeDrawer->currentEndPosition == position) {
       drawConnector(pin);
    }
    
}

void GraphViewComponent::drawConnector(NodeComponent::PinComponent* pin) {
    
    if (edgeDrawer->startPin && edgeDrawer->startPin != pin) {
        
        edgeDrawer->endPin = pin;
        
        if (isLegalEdge(edgeDrawer->startPin, edgeDrawer->endPin)) {
            addEdge(edgeDrawer->startPin, edgeDrawer->endPin);
        }
        
    }
}

bool GraphViewComponent::isLegalEdge(NodeComponent::PinComponent* start, NodeComponent::PinComponent* end) {
    auto existing = find_if(std::begin(edges), std::end(edges), [&](auto& e) -> bool { return e->isConnecting(start, end); });
    return existing == std::end(edges) && start->model->pinType != end->model->pinType && start->node != end->node;
}

void GraphViewComponent::nodeMouseDrag(NodeComponent* node, const MouseEvent& e) {
    
    node->selected = true;
    node->repaint();
    
    if (!e.mods.isShiftDown() && !nodeMultiSelectionOn) {
        for (auto& n : nodes) {
            if (n.get() != node) {
                n->selected = false;
                n->repaint();
            }
        }
    }

    
    auto offset = e.getOffsetFromDragStart();
    node->setTransform(node->scale.followedBy(node->translation.translated(offset * node->scaleFactor)));
    
    for (auto& n : nodes) {
        if (n.get() == node || !(n->selected)) continue;
        
        // using node->scaleFactor here is not a mistake, we need to move according to the dragged node scale
        n->setTransform(n->scale.followedBy(n->translation.translated(offset * node->scaleFactor)));
        
    }
    
    for (auto& e : edges) {
        calculateEdgeBounds(e.get());
        e->repaint();
    }
    
}

void GraphViewComponent::nodeMouseDown(NodeComponent* node, const MouseEvent& e) {
    
    recordState();
    
    if (!e.mods.isShiftDown() && nodeMultiSelectionOn && !node->selected) {
        nodeMultiSelectionOn = false;
    }
    
    node->selected = !node->selected;
    node->repaint();
    
    
    if (!e.mods.isShiftDown() && !nodeMultiSelectionOn) {
        for (auto& n : nodes) {
            if (n.get() != node) {
                n->selected = false;
                n->repaint();
            }
        }
        
        ////
        for (auto& e : edges) {
            e->selected = false;
            e->repaint();
        }
        ////
        
    } else {
        nodeMultiSelectionOn = true;
    }
    
}

void GraphViewComponent::nodeMouseUp(NodeComponent* node, const MouseEvent& e) {
    recordState();
}

void GraphViewComponent::edgeMouseDown(EdgeComponent* edge, const MouseEvent& e) {
    
    if (!e.mods.isShiftDown() && edgeMultiSelectionOn && !edge->selected) {
        edgeMultiSelectionOn = false;
    }
    
    edge->selected = !edge->selected;
    edge->repaint();
    
    if (!e.mods.isShiftDown() && !edgeMultiSelectionOn) {
        
        for (auto& e : edges) {
            if (e.get() != edge) {
                e->selected = false;
                e->repaint();
            }
        }
        
        ////
        for (auto& n : nodes) {
            n->selected = false;
            n->repaint();
        }
        ////
        
    } else {
        edgeMultiSelectionOn = true;
    }
}


void GraphViewComponent::calculateEdgeBounds(EdgeComponent* edge) {
    
    auto startPin = edge->startPin;
    auto endPin = edge->endPin;
    
    auto startPinTopLeft = getLocalPoint(startPin, Point<int>(0, 0));
    auto endPinTopLeft = getLocalPoint(endPin, Point<int>(0, 0));
    
    auto startPinBottomRight = getLocalPoint(startPin, Point<int>(startPin->getWidth(), startPin->getHeight()));
    auto endPinBottomRight = getLocalPoint(endPin, Point<int>(endPin->getWidth(), endPin->getHeight()));
    
    
    auto left = std::min(startPinTopLeft.x, endPinTopLeft.x);
    auto right = std::max(startPinBottomRight.x, endPinBottomRight.x);
    auto top = std::min(startPinTopLeft.y, endPinTopLeft.y);
    auto bottom = std::max(startPinBottomRight.y, endPinBottomRight.y);
    
    auto w = right - left;
    auto h = bottom - top;
    
    edge->inverted = !( left == startPinTopLeft.x );
    
    
    edge->setBounds(left, top, w, h);
}

void GraphViewComponent::edgeMouseUp(EdgeComponent* edge, const MouseEvent& e) {
    
}

void GraphViewComponent::recordState() {
    
    for (auto& n : nodes) {
        auto p = getLocalPoint(n.get(), Point<int>(0,0));
        n->translation = AffineTransform::translation(p);
        n->scale = AffineTransform::scale(n->scaleFactor);
    }
    
}

void GraphViewComponent::mouseDown(const MouseEvent& e) {
    
    if (e.mods.isPopupMenu()) {
        popupMenu(e);
        return;
    }
    
    // node selection
    for (auto& n: nodes) {
        n->selected = false;
        n->repaint();
    }
    nodeMultiSelectionOn = false;
    
    // edge selection
    for (auto& e: edges) {
        e->selected = false;
        e->repaint();
    }
    edgeMultiSelectionOn = false;
    
    // selector
    auto position = e.getMouseDownPosition();
    selector->setBounds(position.x, position.y, 10, 10);
    addAndMakeVisible(selector.get());
}

void GraphViewComponent::mouseDrag(const MouseEvent& e) {
    
    // draw a selection box
    auto position = e.getMouseDownPosition();
    auto offset = e.getOffsetFromDragStart();
    
    
    selector->calculateBounds(position, offset);
    
    // handle node selection
    auto numSelectedNodes = 0;
    for (auto& n : nodes) {
        auto selected = selector->getBounds().intersects(n->getBounds().transformedBy(n->getTransform()));
        n->selected = selected;
        n->repaint();
        if (selected) ++numSelectedNodes;
    }
    
    nodeMultiSelectionOn = numSelectedNodes > 0;
    
    
    // handle edge selection
    auto numSelectedEdges = 0;
    for (auto& e : edges) {
        auto intersection = selector->getBounds().getIntersection(e->getBounds());
        auto selected = intersection == e->getBounds() || intersection.getWidth() >= e->getBounds().getWidth();
        e->selected = selected;
        e->repaint();
        if (selected) ++numSelectedEdges;
    }
    
    edgeMultiSelectionOn = numSelectedEdges > 0;

}

void GraphViewComponent::mouseUp(const MouseEvent& e) {
    removeChildComponent(selector.get());
}


bool GraphViewComponent::keyPressed(const KeyPress& key) {
    
    auto code = key.getKeyCode();
    auto commandDown = key.getModifiers().isCommandDown();
    
    if (code == KeyPress::deleteKey || code == KeyPress::backspaceKey) {
        removeSelected();
    }
    
    // cmd '0'
    else if (code == 48 && commandDown) {
        zoomToOrginalSize();
    }
    
    // cmd '+' or cmd '='
    else if ( (code == 43 || code == 61) && commandDown) {
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
    
    
    return true;
}

void GraphViewComponent::zoomIn() {
    
    recordState();
    
    for (auto& n : nodes) {
        if (n->selected && n->scaleFactor <= theme.scaleMax) {
            n->scaleFactor += theme.scaleStep;
            n->scale = AffineTransform::scale(n->scaleFactor);
            n->setTransform(n->scale.followedBy(n->translation));
        }
    }
    
    for (auto& e : edges) {
        calculateEdgeBounds(e.get());
        e->repaint();
    }
}

void GraphViewComponent::zoomOut() {
    
    recordState();
    
    for (auto& n : nodes) {
        if (n->selected && n->scaleFactor >= theme.scaleMin) {
            n->scaleFactor -= theme.scaleStep;
            n->scale = AffineTransform::scale(n->scaleFactor);
            n->setTransform(n->scale.followedBy(n->translation));
        }
        
    }
    
    for (auto& e : edges) {
        calculateEdgeBounds(e.get());
        e->repaint();
    }
}

void GraphViewComponent::zoomToOrginalSize() {
    
    recordState();
    
    for (auto& n : nodes) {
        if (n->selected) {
            n->scaleFactor = theme.initialScaleFactor;
            n->scale = AffineTransform::scale(n->scaleFactor);
            n->setTransform(n->scale.followedBy(n->translation));
        }
    }
    
    for (auto& e : edges) {
        calculateEdgeBounds(e.get());
        e->repaint();
    }
}

void GraphViewComponent::removeSelected() {
    
    // remove edges first
    std::vector<EdgeComponent*> edgesToDelete;
    for (auto& e : edges) {
        if (e->selected) {
            edgesToDelete.push_back(e.get());
        }
    }
    for (auto& e: edgesToDelete) removeEdge(e);
    nodeMultiSelectionOn = false;
    
    // then remove nodes
    std::vector<NodeComponent*> nodesToDelete;
    for (auto& n : nodes) {
        if (n->selected) {
            nodesToDelete.push_back(n.get());
        }
    }
    for (auto& n: nodesToDelete) removeNode(n);
    edgeMultiSelectionOn = false;
}

void GraphViewComponent::selectAll() {
    
    for (auto& n : nodes) {
        n->selected = true;
    }
    nodeMultiSelectionOn = true;
    
    for (auto& e : edges) {
        e->selected = true;
    }
    edgeMultiSelectionOn = true;
    
    repaint();
}

void GraphViewComponent::duplicate() {
    
    std::vector<std::tuple<NodeComponent*, int, int>> nodesToAdd;
    
    for (auto& n : nodes) {
        if (n->selected) {
            nodesToAdd.push_back(std::tuple<NodeComponent*, int, int>(n.get(), n->ins.size(), n->outs.size()));
        }
    }
    
    for (auto& t : nodesToAdd) {
        NodeComponent* n;
        int ins = 0, outs = 0;
        std::tie(n, ins, outs) = t;
        if (dynamic_cast<HostNodeComponent*>(n)) {
            printf("not supported for now\n");
        } else {
            addNode(n->model->name + "-copy", ins, outs);
        }
        
    }
    
    
}










