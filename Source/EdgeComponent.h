

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Graph.h"
#include "NodeComponent.h"


class EdgeComponent : public Component {
    
public:
    
    GraphViewTheme theme;
    NodeComponent::PinComponent* startPin;
    NodeComponent::PinComponent* endPin;
    const Graph::Edge* model;
    Path path;
    
    bool inverted = false;
    bool selected = false;
    
    EdgeComponent(const GraphViewTheme& theme, NodeComponent::PinComponent* startPin, NodeComponent::PinComponent* endPin, const Graph::Edge* model);
    
    ~EdgeComponent();
    
    bool hitTest(int x, int y) override;
    
    void paint(Graphics& g) override;
        
    bool isConnecting(NodeComponent::PinComponent* first, NodeComponent::PinComponent* second);
    
    
private:
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EdgeComponent)
    
    
};
