#pragma once
#include "JuceHeader.h"

struct ConstrainedComponent : public juce::Component {
  juce::ComponentBoundsConstrainer m_constrains;
};