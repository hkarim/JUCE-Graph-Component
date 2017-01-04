#pragma once

#include "NodeComponent.h"
#include "GraphNodeEditor.h"

class HostNodeComponent : public NodeComponent {

public:
    GraphNodeEditor* editor;
    
    HostNodeComponent(const GraphViewTheme& theme, Graph::Node* model, GraphNodeEditor* editor);
    
    ~HostNodeComponent();
    
    void paint(Graphics& g) override;
    
    void resized() override;
    
    virtual void onData(const Graph::Node* sourceNode, const Graph::Pin* sourcePin, const var& data) override;
    
private:
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HostNodeComponent)
    
};
