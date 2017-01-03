#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class SelectionComponent : public Component {
public:
    
    unsigned int cSelectionBackground;
        
    SelectionComponent(unsigned int backgroundColor);
    
    ~SelectionComponent();
    
    void paint(Graphics& g) override;
    
    void calculateBounds(const Point<int>& start, const Point<int>& offset);
    
private:
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SelectionComponent)
    
};
