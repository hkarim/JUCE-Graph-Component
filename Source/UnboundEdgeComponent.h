
#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "NodeComponent.h"

class UnboundEdgeComponent : public Component {
    
public:
    
    bool inverted = false;
    
    NodeComponent::PinComponent* startPin;
    NodeComponent::PinComponent* endPin;
    Point<int> currentEndPosition;

    
    UnboundEdgeComponent();
    
    void paint(Graphics& g) override;
    
    void calculateBounds(const Point<int>& start, const Point<int>& end);
    
private:
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UnboundEdgeComponent)
};
