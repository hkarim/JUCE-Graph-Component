#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Graph.h"

class GraphNodeEditor {
public:
    
    virtual ~GraphNodeEditor() {}
    
    virtual Component* guest() = 0;
    
    virtual void setModel(Graph::Node* model) {}
    
    virtual void onData(const Graph::Node* sourceNode, const Graph::Pin* sourcePin, const Graph::Data& data) {}
    
};
