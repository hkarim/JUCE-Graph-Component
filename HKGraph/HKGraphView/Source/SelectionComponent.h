#pragma once

#include "JuceHeader.h"

class SelectionComponent : public juce::Component {
public:
    unsigned int cSelectionBackground;
    explicit SelectionComponent(unsigned int backgroundColor);
    ~SelectionComponent() override;
    void paint(juce::Graphics& g) override;
    void calculateBounds(const juce::Point<int>& start, const juce::Point<int>& offset);
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SelectionComponent)
};
