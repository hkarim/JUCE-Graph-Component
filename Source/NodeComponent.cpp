

#include "NodeComponent.h"


NodeComponent::NodeComponent(const GraphViewTheme& theme, Graph::Node* model) : theme(theme), model(model) {
    
    model->addListener(this);
    
    cNodeBackgroundCurrent = theme.cNodeBackground;
    
    for(auto i = 0; i < model->ins.size(); i++) {
        auto pin = std::make_unique<PinComponent>(this, model->ins[i].get());
        ins.push_back(std::move(pin));
    }
    
    for(auto i = 0; i < model->outs.size(); i++) {
        auto pin = std::make_unique<PinComponent>(this, model->outs[i].get());
        outs.push_back(std::move(pin));
    }
    
    translation = AffineTransform::translation(0, 0);
    
    scaleFactor = theme.initialScaleFactor;
    scale = AffineTransform::scale(scaleFactor);
    
}

NodeComponent::~NodeComponent() {
    model->removeListener(this);
}

bool NodeComponent::hasIns() {
    return ins.size() > 0;
}

bool NodeComponent::hasOuts() {
    return outs.size() > 0;
}

int NodeComponent::boxHeight() {
    auto h = getHeight();
    if (hasIns()) h -= theme.pinHeight;
    if (hasOuts()) h -= theme.pinHeight;
    return h;
}

Rectangle<int> NodeComponent::boxBounds() {
    auto local = getLocalBounds();
    if (hasIns()) local.removeFromTop(theme.pinHeight);
    if (hasOuts()) local.removeFromBottom(theme.pinHeight);
    return local;
}

void NodeComponent::paint(Graphics& g) {
    Path p;
    auto bounds = boxBounds();
    //p.addRectangle(bounds);
    p.addRoundedRectangle(bounds, 3);
    
    if (selected)
        g.setColour(Colour(theme.cNodeBackgroundSelected));
    else
        g.setColour(Colour(cNodeBackgroundCurrent));
    
    g.fillPath(p);
    g.setColour(Colours::white);
    g.drawText(model->name, boxBounds(), Justification::centred, true);
}

void NodeComponent::resized() {
    
    {
        auto x = theme.pinSpacing;
        auto y = 0;
        auto w = theme.pinWidth;
        auto h = theme.pinHeight;
        for (auto& p : ins) {
            p->setBounds(x, y, w, h);
            addAndMakeVisible(p.get());
            x += w + theme.pinSpacing;
        }
    }
    
    {
        auto x = theme.pinSpacing;
        auto y = getHeight() - theme.pinHeight;
        auto w = theme.pinWidth;
        auto h = theme.pinHeight;
        for (auto& p : outs) {
            p->setBounds(x, y, w, h);
            addAndMakeVisible(p.get());
            x += w + theme.pinSpacing;
        }
    }
    
}

bool NodeComponent::hitTest(int x, int y) {
    auto inBox = boxBounds().contains(x, y);
    auto inTopPins = false;
    auto inBottomPins = false;
    
    for (auto& p : ins) {
        if (p->getBounds().contains(x, y)) {
            inTopPins = true;
            break;
        }
    }
    
    for (auto& p : outs) {
        if (p->getBounds().contains(x, y)) {
            inBottomPins = true;
            break;
        }
    }
    
    return inBox || inTopPins || inBottomPins;
}

void NodeComponent::mouseDown(const MouseEvent& e) {
}

void NodeComponent::mouseDrag(const MouseEvent& e) {
    
    
}

void NodeComponent::mouseUp(const MouseEvent& e) {
}


void NodeComponent::mouseMove(const MouseEvent& e) {
    if (boxBounds().contains(e.x, e.y)) {
        cNodeBackgroundCurrent = theme.cNodeBackgroundHover;
        repaint();
    }
}


void NodeComponent::mouseExit(const MouseEvent& e) {
    cNodeBackgroundCurrent = theme.cNodeBackground;
    repaint();
}


void NodeComponent::mouseDoubleClick(const MouseEvent& e) {
    //model->flow(model->name);
    //model->graph->dfs(model, [&](const auto& n) -> void { printf("visiting %s\n", n->name.c_str()); } );
}

















