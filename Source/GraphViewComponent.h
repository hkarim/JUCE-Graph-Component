

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "NodeComponent.h"
#include "HostNodeComponent.h"
#include "EdgeComponent.h"
#include "SelectionComponent.h"
#include "UnboundEdgeComponent.h"
#include "GraphNodeEditor.h"


class GraphViewComponent : public Component {
    
public:
    
    std::unique_ptr<Graph> graph;
    std::vector<std::unique_ptr<NodeComponent>> nodes;
    std::vector<std::unique_ptr<EdgeComponent>> edges;
    std::unique_ptr<SelectionComponent> selector;
    std::unique_ptr<UnboundEdgeComponent> edgeDrawer;
    
    bool nodeMultiSelectionOn = false;
    bool edgeMultiSelectionOn = false;
        
    
    GraphViewTheme theme;
    
    struct ChildrenMouseListener : public MouseListener {
        
        GraphViewComponent* view;
        
        ChildrenMouseListener(GraphViewComponent* view) : view(view) { }
        
        void mouseDown(const MouseEvent& e) override {
            
            if (auto node = dynamic_cast<NodeComponent*>(e.originalComponent)) {
                view->nodeMouseDown(node, e);
            } else if (auto edge = dynamic_cast<EdgeComponent*>(e.originalComponent)) {
                view->edgeMouseDown(edge, e);
            }
                
        }
        
        void mouseUp(const MouseEvent& e) override {
            
            if (auto node = dynamic_cast<NodeComponent*>(e.originalComponent)) {
                view->nodeMouseUp(node, e);
            } else if (auto pin = dynamic_cast<NodeComponent::PinComponent*>(e.originalComponent)) {
                view->pinMouseUp(pin, e);
            } else if (auto edge = dynamic_cast<EdgeComponent*>(e.originalComponent)) {
                view->edgeMouseUp(edge, e);
            }
            
                
        }
        
        void mouseDrag(const MouseEvent& e) override {
            
            if (auto node = dynamic_cast<NodeComponent*>(e.originalComponent)) {
                view->nodeMouseDrag(node, e);
            } else if (auto pin = dynamic_cast<NodeComponent::PinComponent*>(e.originalComponent)) {
                view->pinMouseDrag(pin, e);
            }
            
        }
        
        void mouseEnter(const MouseEvent& e) override {
            
            if (auto pin = dynamic_cast<NodeComponent::PinComponent*>(e.originalComponent)) {
                view->pinMouseEnter(pin, e);
            }
            
        }
    };
    
    std::unique_ptr<ChildrenMouseListener> mouseListener;
    
    
    GraphViewComponent();
    
    ~GraphViewComponent();
    
    void paint(Graphics&) override;
    
    void resized() override;
    
    
    NodeComponent* addNode(
                           const std::string& name,
                           const int ins,
                           const int outs,
                           const Point<float> position = Point<float>(0,0));
    HostNodeComponent* addHostNode(
                                   GraphNodeEditor* editor,
                                   const int ins,
                                   const int outs,
                                   const int width,
                                   const int height,
                                   const Point<float> position = Point<float>(0,0));
    void removeNode(NodeComponent* n);
    
    EdgeComponent* addEdge(NodeComponent::PinComponent* source, NodeComponent::PinComponent* target);
    void removeEdge(EdgeComponent* e);
    
    void assertions() const;
        
    void pinMouseDrag(NodeComponent::PinComponent* pin, const MouseEvent& e);
    void pinMouseUp(NodeComponent::PinComponent* pin, const MouseEvent& e);
    void pinMouseEnter(NodeComponent::PinComponent* pin, const MouseEvent& e);
    
    void drawConnector(NodeComponent::PinComponent* pin);
    
    bool isLegalEdge(NodeComponent::PinComponent* start, NodeComponent::PinComponent* end);
    
    void nodeMouseDown(NodeComponent* node, const MouseEvent& e);
    void nodeMouseUp(NodeComponent* node, const MouseEvent& e);
    void nodeMouseDrag(NodeComponent* node, const MouseEvent& e);
    
    void calculateEdgeBounds(EdgeComponent* edge);
    void edgeMouseDown(EdgeComponent* edge, const MouseEvent& e);
    void edgeMouseUp(EdgeComponent* edge, const MouseEvent& e);
    
    void mouseDown(const MouseEvent& e) override;
    
    void mouseDrag(const MouseEvent& e) override;
    
    void mouseUp(const MouseEvent& e) override;
    
    virtual void popupMenu(const MouseEvent& e) {};
    
    bool keyPressed(const KeyPress& key) override;
    
    void recordState();
    
    void zoomIn();
    void zoomOut();
    void zoomToOrginalSize();
    
    void selectAll();
    
    void removeSelected();
    
    void duplicate();
    
    
private:
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GraphViewComponent)
};


