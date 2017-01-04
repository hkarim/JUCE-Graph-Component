
#include "HostNodeComponent.h"


HostNodeComponent::HostNodeComponent(const GraphViewTheme& theme, Graph::Node* model, std::unique_ptr<GraphNodeEditor> editor) :
NodeComponent(theme, model), editor(std::move(editor)) {
    
    this->editor->setModel(model);
    
}

HostNodeComponent::~HostNodeComponent() {
    
}


void HostNodeComponent::paint(Graphics& g) {
    Path p;
    auto bounds = boxBounds();
    p.addRoundedRectangle(bounds, 3);
    
    if (selected)
        g.setColour(Colour(theme.cNodeBackgroundSelected));
    else
        g.setColour(Colour(cNodeBackgroundCurrent));
    
    g.fillPath(p);
}


void HostNodeComponent::resized() {
    auto bounds = boxBounds();
    auto guest = editor->guest();
    bounds.removeFromTop(theme.hostComponentDragAreaHeight);
    guest->setBounds(0, theme.hostComponentDragAreaHeight, bounds.getWidth(), bounds.getHeight() + theme.pinHeight);
    addAndMakeVisible(guest);
    NodeComponent::resized();
}

void HostNodeComponent::onData(const Graph::Node* sourceNode, const Graph::Pin* sourcePin, const var& data) {
    editor->onData(sourceNode, sourcePin, data);
};





