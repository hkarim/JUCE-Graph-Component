#pragma once

#include "JuceHeader.h"
#include "NodeComponent.h"
#include "EdgeComponent.h"
#include "SelectionComponent.h"
#include "UnboundEdgeComponent.h"

class GraphViewComponent : public juce::Component, public juce::ScrollBar::Listener {

public:
  Graph *graph;
  std::unordered_map<uuid, NodeComponent *> nodes;
  std::unordered_map<uuid, EdgeComponent *> edges;
  std::unique_ptr<SelectionComponent> selector;
  std::unique_ptr<UnboundEdgeComponent> edgeDrawer;

  bool nodeMultiSelectionOn = false;
  bool edgeMultiSelectionOn = false;
  GraphViewTheme theme;
  juce::ScrollBar hsb{false};
  int hsbLast{};
  juce::ScrollBar vsb{true};
  int vsbLast{};

  explicit GraphViewComponent(Graph *sharedGraph);

  struct ChildrenMouseListener : public MouseListener {

    GraphViewComponent *view;

    explicit ChildrenMouseListener(GraphViewComponent *graphViewComponent) : view(graphViewComponent) {}

    void mouseDown(const juce::MouseEvent &e) override {
      if (auto node = dynamic_cast<NodeComponent *>(e.originalComponent)) {
        view->nodeMouseDown(node, e);
      } else if (auto edge = dynamic_cast<EdgeComponent *>(e.originalComponent)) {
        view->edgeMouseDown(edge, e);
      }
    }

    void mouseUp(const juce::MouseEvent &e) override {
      if (auto node = dynamic_cast<NodeComponent *>(e.originalComponent)) {
        view->nodeMouseUp(node, e);
      } else if (auto pin = dynamic_cast<NodeComponent::PinComponent *>(e.originalComponent)) {
        view->pinMouseUp(pin, e);
      } else if (auto edge = dynamic_cast<EdgeComponent *>(e.originalComponent)) {
        view->edgeMouseUp(edge, e);
      }
    }

    void mouseDrag(const juce::MouseEvent &e) override {
      if (auto node = dynamic_cast<NodeComponent *>(e.originalComponent)) {
        view->nodeMouseDrag(node, e);
      } else if (auto pin = dynamic_cast<NodeComponent::PinComponent *>(e.originalComponent)) {
        view->pinMouseDrag(pin, e);
      } else if (auto edge = dynamic_cast<EdgeComponent *>(e.originalComponent)) {
        view->edgeMouseDrag(edge, e);
      }
    }

    void mouseEnter(const juce::MouseEvent &e) override {
      if (auto pin = dynamic_cast<NodeComponent::PinComponent *>(e.originalComponent)) {
        view->pinMouseEnter(pin, e);
      }
    }

    void mouseDoubleClick(const juce::MouseEvent &e) override {
      if (auto node = dynamic_cast<NodeComponent *>(e.originalComponent)) {
        view->nodeMouseDoubleClick(node, e);
      } else if (auto edge = dynamic_cast<EdgeComponent *>(e.originalComponent)) {
        view->edgeMouseDoubleClick(edge, e);
      }
    }


  };

  std::unique_ptr<ChildrenMouseListener> mouseListener;

  ~GraphViewComponent() override;

  void paint(juce::Graphics &) override;

  void resized() override;

  void scrollBarMoved(juce::ScrollBar *scrollBarThatHasMoved, double newRangeStart) override;

  void recordUI(std::unordered_map<uuid, std::unique_ptr<NodeDescriptor>> &nodeDescriptors);

  void restoreUI(std::unordered_map<uuid, std::unique_ptr<NodeDescriptor>> &nodeDescriptors);

  void addNode(
    NodeProcessor *processor,
    const juce::Point<float> &position);

  void addNodeSized(
    NodeProcessor *processor,
    int width,
    int height,
    const juce::Point<float> &position);

  void addHostNode(
    NodeProcessor *processor,
    int width,
    int height,
    const juce::Point<float> &position);

  NodeComponent *clone(const NodeComponent *original);

  std::optional<EdgeComponent *> addEdge(NodeComponent::PinComponent *source, NodeComponent::PinComponent *target);

  void assertions() const;

  void pinMouseDrag(NodeComponent::PinComponent *pin, const juce::MouseEvent &e);

  void pinMouseUp(NodeComponent::PinComponent *pin, const juce::MouseEvent &e);

  void pinMouseEnter(NodeComponent::PinComponent *pin, const juce::MouseEvent &e);

  void drawConnector(NodeComponent::PinComponent *pin);

  bool isLegalEdge(NodeComponent::PinComponent *start, NodeComponent::PinComponent *end);

  void nodeMouseDown(NodeComponent *node, const juce::MouseEvent &e);

  void nodeMouseUp(NodeComponent *node, const juce::MouseEvent &e);

  void nodeMouseDrag(NodeComponent *node, const juce::MouseEvent &e);

  void nodeMouseDoubleClick(NodeComponent *node, const juce::MouseEvent &e);

  void calculateEdgeBounds(EdgeComponent *edge);

  void edgeMouseDown(EdgeComponent *edge, const juce::MouseEvent &e);

  void edgeMouseDrag(EdgeComponent *edge, const juce::MouseEvent &e);

  void edgeMouseUp(EdgeComponent *edge, const juce::MouseEvent &e);

  void edgeMouseDoubleClick(EdgeComponent *edge, const juce::MouseEvent &e);

  void mouseDown(const juce::MouseEvent &mouseEvent) override;

  void mouseDrag(const juce::MouseEvent &mouseEvent) override;

  void mouseUp(const juce::MouseEvent &e) override;

  void mouseWheelMove(const juce::MouseEvent &, const juce::MouseWheelDetails &wheel) override {
    if (wheel.deltaX != 0.0f) {
      hsb.setCurrentRangeStart(hsb.getCurrentRangeStart() + wheel.deltaX * 10.0f);
    }
    if (wheel.deltaY != 0.0f) {
      vsb.setCurrentRangeStart(vsb.getCurrentRangeStart() + wheel.deltaY * 10.0f);
    }
  }

  void childBoundsChanged(juce::Component *child) override;

  virtual void popupMenu(const juce::MouseEvent &e) {
    juce::ignoreUnused(e);
  }

  bool keyPressed(const juce::KeyPress &key) override;

  void recordState();

  void zoomIn();

  void zoomOut();

  void zoomToOriginalSize();

  void selectAll();

  void removeSelected();

  void duplicate();

  void mute();

  void debug(const std::string &action) const;

private:

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GraphViewComponent)
};
