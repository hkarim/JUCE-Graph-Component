#pragma once

#include "JuceHeader.h"

namespace ce {
  template<typename T>
  struct CurveEditorModel {
    using NumericType = T;
    using PointType = juce::Point<T>;

    enum class CurveType {
      Linear = 0,
      Quadratic,
      Cubic
    };

    static constexpr int CurveTypeCount = 3;

    struct Handle;

    struct Node {
      Handle anchor;
      Handle control1;
      Handle control2;
      CurveType curveType = CurveType::Linear;

      explicit Node(const PointType &anchorPoint) :
        anchor(Handle{anchorPoint, this}),
        control1(Handle{anchorPoint, this}),
        control2(Handle{anchorPoint, this}) {}

      void setAnchorPt(const PointType &pt) {
        auto anchorControlDist1 = anchor.pt - control1.pt;
        auto anchorControlDist2 = anchor.pt - control2.pt;
        anchor.pt = pt;
        control1.pt = pt - anchorControlDist1;
        control2.pt = pt - anchorControlDist2;
      }

      void setControlPt1(const PointType &pt) {
        control1.pt = pt;
      }

      void setControlPt2(const PointType &pt) {
        control2.pt = pt;
      }

      [[nodiscard]] juce::ValueTree toValueTree(const juce::Identifier &id) const {
        return {
          id, {
            {"curveType", static_cast<int> (curveType)}
          },
          {
            {"anchor", {{"x", anchor.pt.x}, {"y", anchor.pt.y}}},
            {"control1", {{"x", control1.pt.x}, {"y", control1.pt.y}}},
            {"control2", {{"x", control2.pt.x}, {"y", control2.pt.y}}}
          }
        };
      }
    };

    struct Handle {
      PointType pt;
      Node *parent;

      Handle(const PointType &point, Node *parentNode) :
        pt(point),
        parent(parentNode) {}

      void setX(T newX) { pt.setX(newX); }

      //void setY(T newY) { pt.setY(newY); }
    };

    explicit CurveEditorModel(T _minX, T _maxX, T _minY, T _maxY) :
      minX(_minX),
      maxX(_maxX),
      minY(_minY),
      maxY(_maxY),
      lastInputValue(static_cast<T> (0)) {
      nodes.clear();
      nodes.emplace_back(std::make_shared<Node>(PointType{minX, minY}));
      nodes.emplace_back(std::make_shared<Node>(PointType{
        minX + (maxX - minX) * static_cast<T> (0.5),
        minY + (maxY - minY) * static_cast<T> (0.5)
      }));
      nodes.emplace_back(std::make_shared<Node>(PointType{maxX, maxY}));
    }

    void fromValueTree(const juce::ValueTree &tree) {
      if (tree.getNumChildren()) {
        nodes.clear();
        for (int i = 0; i < tree.getNumChildren(); i++) {
          auto child = tree.getChild(i);
          auto anchor = child.getChildWithName("anchor");
          auto control1 = child.getChildWithName("control1");
          auto control2 = child.getChildWithName("control2");
          auto node = std::make_shared<Node>(PointType{anchor.getProperty("x"), anchor.getProperty("y")});
          node->curveType = static_cast<CurveType> (static_cast<int> (child.getProperty("curveType")));
          node->setControlPt1(PointType{control1.getProperty("x"), control1.getProperty("y")});
          node->setControlPt2(PointType{control2.getProperty("x"), control2.getProperty("y")});
          nodes.push_back(node);
        }
      }
    }

    /**
     * Map an input value onto the curve
     */
    T compute(T input);

    T minX, maxX;
    T minY, maxY;
    std::vector<std::shared_ptr<Node>> nodes;
    juce::Value lastInputValue;
  };

  template<typename T>
  T CurveEditorModel<T>::compute(T input) {
    jassert (nodes.size() > 1);
    for (size_t i = 1; i < nodes.size(); i++) {
      const auto &lastNode = *nodes[i - 1];
      const auto &node = *nodes[i];

      const auto &lastAnchorPoint = lastNode.anchor.pt;
      const auto &anchorPoint = node.anchor.pt;

      jassert (lastAnchorPoint.x <= anchorPoint.x);

      if (input <= anchorPoint.x) {
        auto computeCubic = [](const PointType &p0, const PointType &p1, const PointType &p2, const PointType &p3,
                               auto t) {
          return p0 * std::pow(1 - t, 3.0) + p1 * 3 * std::pow(1 - t, 2.0) * t + p2 * 3 * (1 - t) * std::pow(t, 2.0) +
                 p3 *
                 std::pow(t, 3.0);
        };

        auto computeQuadratic = [](const PointType &p0, const PointType &p1, const PointType &p2, auto t) {
          return p0 * std::pow(1 - t, 2.0) + p1 * 2 * (1 - t) * t + p2 * std::pow(t, 2.0);
        };

        switch (lastNode.curveType) {
          case CurveType::Cubic: {
            const auto &ctrlPoint1 = lastNode.control1.pt;
            const auto &ctrlPoint2 = lastNode.control2.pt;
            float finalT = 0.0f;
            float minDist = -1.0f;
            for (int j = 0; j <= 100; j++) {
              const float t = static_cast<float> (j) / 100.0f;
              const float x = computeCubic(lastAnchorPoint, ctrlPoint1, ctrlPoint2, anchorPoint, t).x;
              const float distance = std::abs(x - input);
              if (distance < minDist || minDist < 0.0f) {
                finalT = t;
                minDist = distance;
              }
            }
            return computeCubic(lastAnchorPoint, ctrlPoint1, ctrlPoint2, anchorPoint, finalT).y;
          }
          case CurveType::Quadratic: {
            const auto &ctrlPoint = lastNode.control1.pt;
            float finalT = 0.0f;
            float minDist = -1.0f;
            for (int j = 0; j <= 100; j++) {
              const float t = static_cast<float> (j) / 100.0f;
              const float x = computeQuadratic(lastAnchorPoint, ctrlPoint, anchorPoint, t).x;
              const float distance = std::abs(x - input);
              if (distance < minDist || minDist < 0.0f) {
                finalT = t;
                minDist = distance;
              }
            }
            return computeQuadratic(lastAnchorPoint, ctrlPoint, anchorPoint, finalT).y;
          }
          default:
          case CurveType::Linear:
            const auto slope = (anchorPoint.y - lastAnchorPoint.y) / (anchorPoint.x - lastAnchorPoint.x);
            return slope * (input - lastAnchorPoint.x) + lastAnchorPoint.y;
        }
      }
    }
    jassertfalse; // TODO
    return 0.0f;
  }

  template<typename T>
  class CurveEditor : public juce::Component, juce::Value::Listener {
    using PointType = typename CurveEditorModel<T>::PointType;
    using Handle = typename CurveEditorModel<T>::Handle;
    using Node = typename CurveEditorModel<T>::Node;
    using CurveType = typename CurveEditorModel<T>::CurveType;
  public:
    explicit CurveEditor(CurveEditorModel<T> &m) :
      model(m) {
      lastInputValue.referTo(model.lastInputValue);
      lastInputValue.addListener(this);
    }

    ~CurveEditor() override {
      lastInputValue.removeListener(this);
    }

    void drawReadableSingleLineText(juce::Graphics &g, const typename CurveEditorModel<T>::PointType &baseline,
                                    const std::string &text,
                                    int yThreshold = 25, int xThreshold = 25);

    void paint(juce::Graphics &g) override;

    void mouseDown(const juce::MouseEvent &event) override;

    void mouseDrag(const juce::MouseEvent &event) override;

    void mouseUp(const juce::MouseEvent &event) override;

    void mouseDoubleClick(const juce::MouseEvent &event) override;

    void mouseMove(const juce::MouseEvent &event) override;

    void resized() override;

    void valueChanged(juce::Value &value) override;

    void addPoint(const PointType &p);

    /**
     * \brief Get a reference to the handle closest to the given point (in screen space)
     */
    Handle *getClosestHandle(const PointType &screenPt, T thresholdDistance = -1);

    std::shared_ptr<Node> getNextNode(const Node &fromNode);

    std::shared_ptr<Node> getPrevNode(const Node &fromNode);

  private:
    PointType transformPointToScreenSpace(const PointType &p) const;

    PointType transformPointFromScreenSpace(const PointType &p) const;

  private:
    const float POINT_SIZE = 10.0f;
    const float DISTANCE_THRESHOLD = POINT_SIZE * 2.0f;
    juce::AffineTransform screenSpaceTransform;
    Handle *selectedHandle = nullptr;
    CurveEditorModel<T> &model;
    juce::Value lastInputValue;
  };

  template<typename T>
  void
  CurveEditor<T>::drawReadableSingleLineText(juce::Graphics &g, const typename CurveEditorModel<T>::PointType &baseline,
                                             const std::string &text, int yThreshold, int xThreshold) {
    juce::Justification textJustification(juce::Justification::right);
    int screenSpaceYOffset = 0;
    int screenSpaceXOffset = 0;
    if (static_cast<int> (baseline.y) < yThreshold) {
      screenSpaceYOffset = static_cast<int> (g.getCurrentFont().getAscent());
      textJustification = juce::Justification::right;
    }
    if (static_cast<int> (baseline.x) < xThreshold) {
      textJustification = juce::Justification::left;
      screenSpaceXOffset = 10;
    } else if (static_cast<int> (baseline.x) > getWidth() - xThreshold) {
      textJustification = juce::Justification::right;
    }
    g.drawSingleLineText(text, static_cast<int> (baseline.x) + screenSpaceXOffset,
                         static_cast<int> (baseline.y) + screenSpaceYOffset,
                         textJustification);
  }

  template<typename T>
  void CurveEditor<T>::paint(juce::Graphics &g) {
    g.setColour(juce::Colours::black);
    g.fillRect(0, 0, getWidth(), getHeight());

    // Record mouse coordinates in screen/model space
    const PointType screenSpaceMousePt = getMouseXYRelative().toFloat();
    const PointType modelSpaceMousePt = transformPointFromScreenSpace(screenSpaceMousePt);

    // Mark handle currently being hovered over
    Handle *hoveredHandle = getClosestHandle(screenSpaceMousePt, DISTANCE_THRESHOLD);

    auto drawHandle = [this, &g, &hoveredHandle](const Handle &handle) {
      auto transformedHandlePoint = transformPointToScreenSpace(handle.pt);
      if (selectedHandle == &handle) {
        g.setColour(juce::Colours::cyan);
        g.fillEllipse(transformedHandlePoint.x - POINT_SIZE * 0.5f,
                      transformedHandlePoint.y - POINT_SIZE * 0.5f,
                      POINT_SIZE, POINT_SIZE);
      } else {
        if (selectedHandle == nullptr && hoveredHandle == &handle) {
          g.setColour(juce::Colours::goldenrod);
          g.drawEllipse(transformedHandlePoint.x - POINT_SIZE * 0.5f,
                        transformedHandlePoint.y - POINT_SIZE * 0.5f,
                        POINT_SIZE, POINT_SIZE, 2.0f);
        } else {
          g.setColour(juce::Colours::darkgoldenrod);
          g.drawEllipse(transformedHandlePoint.x - POINT_SIZE * 0.5f,
                        transformedHandlePoint.y - POINT_SIZE * 0.5f,
                        POINT_SIZE, POINT_SIZE, 1.0f);
        }
      }
      return transformedHandlePoint;
    };

    auto drawHandles = [drawHandle, &g](const Node &node) {
      auto transformedAnchorPoint = drawHandle(node.anchor);

      if (node.curveType == CurveType::Quadratic) {
        auto transformedControlPoint = drawHandle(node.control1);
        g.setColour(juce::Colours::darkgoldenrod);
        g.drawLine(transformedAnchorPoint.x, transformedAnchorPoint.y, transformedControlPoint.x,
                   transformedControlPoint.y);
      } else if (node.curveType == CurveType::Cubic) {
        auto transformedControlPoint1 = drawHandle(node.control1);
        g.setColour(juce::Colours::darkgoldenrod);
        g.drawLine(transformedAnchorPoint.x, transformedAnchorPoint.y, transformedControlPoint1.x,
                   transformedControlPoint1.y);
        auto transformedControlPoint2 = drawHandle(node.control2);
        g.setColour(juce::Colours::darkgoldenrod);
        g.drawLine(transformedAnchorPoint.x, transformedAnchorPoint.y, transformedControlPoint2.x,
                   transformedControlPoint2.y);
      }
    };

    // Draw the actual curve
    juce::Path curve;
    for (size_t i = 0; i < model.nodes.size(); i++) {
      const auto transformedAnchorPoint = transformPointToScreenSpace(model.nodes[i]->anchor.pt);
      if (i == 0)
        curve.startNewSubPath(transformedAnchorPoint);
      else {
        CurveType curve_type = model.nodes[i - 1]->curveType;
        if (curve_type == CurveType::Linear) {
          curve.lineTo(transformedAnchorPoint);
        } else if (curve_type == CurveType::Quadratic) {
          const auto transformedControlPoint1 = transformPointToScreenSpace(model.nodes[i - 1]->control1.pt);
          curve.quadraticTo(transformedControlPoint1.x, transformedControlPoint1.y, transformedAnchorPoint.x,
                            transformedAnchorPoint.y);
        } else if (curve_type == CurveType::Cubic) {
          const auto transformedControlPoint1 = transformPointToScreenSpace(model.nodes[i - 1]->control1.pt);
          const auto transformedControlPoint2 = transformPointToScreenSpace(model.nodes[i - 1]->control2.pt);
          curve.cubicTo(transformedControlPoint1.x, transformedControlPoint1.y, transformedControlPoint2.x,
                        transformedControlPoint2.y, transformedAnchorPoint.x, transformedAnchorPoint.y);
        }
      }
      drawHandles(*model.nodes[i]);
    }

    g.setColour(juce::Colours::whitesmoke);
    g.strokePath(curve, juce::PathStrokeType(1.0f));

    // Draw reference line from the mouse pointer to the curve
    if (!selectedHandle && contains(getMouseXYRelative())) {
      const auto modelSpaceCurvePt = PointType(modelSpaceMousePt.x, model.compute(modelSpaceMousePt.x));
      const auto screenSpaceCurvePt = transformPointToScreenSpace(modelSpaceCurvePt);

      g.setColour(juce::Colours::firebrick);
      g.drawVerticalLine(static_cast<int> (screenSpaceMousePt.x),
                         juce::jmin(screenSpaceCurvePt.y, screenSpaceMousePt.y),
                         juce::jmax(screenSpaceCurvePt.y, screenSpaceMousePt.y));

      // Draw reference text describing point on the curve
      g.setColour(juce::Colours::firebrick);
      std::ostringstream os;
      os << std::fixed << std::setprecision(0) << "[" << modelSpaceCurvePt.x << ", " << modelSpaceCurvePt.y << "]";
      drawReadableSingleLineText(g, screenSpaceMousePt + PointType(0, g.getCurrentFont().getAscent() + 5), os.str(),
                                 25 + static_cast<int>(g.getCurrentFont().getAscent()) + 5);
    }

    // Draw reference text describing cursor coordinate
    {
      g.setColour(juce::Colours::slategrey);
      std::ostringstream os;
      os << std::fixed << std::setprecision(0) << "(" << modelSpaceMousePt.x << ", " << modelSpaceMousePt.y << ")";
      drawReadableSingleLineText(g, screenSpaceMousePt, os.str());
    }

    // Draw reference line for most recent input/output
    {
      g.setColour(juce::Colours::lightblue);
      T inputValue = lastInputValue.getValue();
      T outputValue = model.compute(inputValue);
      const auto screenSpaceCurvePt = transformPointToScreenSpace(PointType(inputValue, outputValue));
      g.drawVerticalLine(static_cast<int> (screenSpaceCurvePt.x), screenSpaceCurvePt.y,
                         static_cast<float> (getHeight()));
      std::ostringstream ostr;
      ostr << std::fixed << std::setprecision(0) << "[" << inputValue << ", " << outputValue << "]";
      drawReadableSingleLineText(g, screenSpaceCurvePt, ostr.str());
    }


    // Draw grid
    auto numXTicks = 10; // TODO: Make these editable parameters
    auto numYTicks = 10;
    const juce::Colour slightWhite = juce::Colour::fromRGBA(255, 255, 255, 50);
    g.setColour(slightWhite);
    for (auto i = 0; i < numXTicks; i++) {
      T currX = (model.maxX - model.minX) / static_cast<T> (numXTicks) * static_cast<T> (i) + model.minX;
      PointType screenX = transformPointToScreenSpace(PointType(currX, 0));
      g.drawVerticalLine(static_cast<int> (screenX.x), 0.0f, static_cast<float> (getHeight()));
    }
    for (auto i = 0; i < numYTicks; i++) {
      T currY = (model.maxY - model.minY) / static_cast<T> (numYTicks) * static_cast<T> (i) + model.minY;
      PointType screenY = transformPointToScreenSpace(PointType(0, currY));
      g.drawHorizontalLine(static_cast<int> (screenY.y), 0.0f, static_cast<float> (getWidth()));
    }
  }

  template<typename T>
  void CurveEditor<T>::mouseDown(const juce::MouseEvent &event) {
    jassert (!model.nodes.empty());

    Handle *closestHandle = getClosestHandle(event.mouseDownPosition, DISTANCE_THRESHOLD);
    if (!closestHandle) {
      selectedHandle = nullptr;
      return;
    }

    //const PointType& closestPt = closestHandle->pt;

    if (event.mods.isLeftButtonDown()) {
      selectedHandle = closestHandle;
    } else if (event.mods.isRightButtonDown()) {
      if (closestHandle->parent != model.nodes.front().get() && closestHandle->parent != model.nodes.back().get()) {
        int toErase = -1;
        for (size_t i = 0; i < model.nodes.size(); i++) {
          if (model.nodes[i].get() == closestHandle->parent) {
            toErase = static_cast<int>(i);
            break;
          }
        }
        if (toErase != -1) {
          model.nodes.erase(model.nodes.begin() + toErase);
        }
        selectedHandle = nullptr;
      }
    }

    repaint();
  }

  template<typename T>
  void CurveEditor<T>::mouseDrag(const juce::MouseEvent &event) {
    if (selectedHandle) {
      const PointType mousePt = event.getPosition().toFloat();
      const PointType modelSpaceMousePt = transformPointFromScreenSpace(mousePt);
      PointType selectedPoint = selectedHandle->pt;
      CurveType curveType = selectedHandle->parent->curveType;

      // Adjust selected point within the X and Y boundaries
      selectedPoint = selectedPoint + 0.9f * (modelSpaceMousePt - selectedPoint);
      selectedPoint.setX(juce::jlimit(model.minX + 1, model.maxX - 1, selectedPoint.getX()));
      selectedPoint.setY(juce::jlimit(model.minY, model.maxY, selectedPoint.getY()));

      // Lock the X position of the first and last anchor points
      if (selectedHandle->parent == model.nodes.front().get() && &selectedHandle->parent->anchor == selectedHandle) {
        selectedPoint.setX(model.minX);
      } else if (selectedHandle->parent == model.nodes.back().get() &&
                 &selectedHandle->parent->anchor == selectedHandle) {
        selectedPoint.setX(model.maxX);
      }

      std::shared_ptr<Node> nextNode = getNextNode(*selectedHandle->parent);
      std::shared_ptr<Node> prevNode = getPrevNode(*selectedHandle->parent);
      if (selectedHandle == &selectedHandle->parent->anchor) {
        // Push handles of prev/next nodes when moving an anchor
        // Limit anchor so it doesn't cross prev/next anchors
        if (prevNode) {
          selectedPoint.setX(juce::jmax(selectedPoint.getX(), prevNode->anchor.pt.getX()));
          selectedHandle->parent->control1.pt.setX(
            juce::jmax(selectedHandle->parent->control1.pt.getX(), prevNode->anchor.pt.getX()));
          selectedHandle->parent->control2.pt.setX(
            juce::jmax(selectedHandle->parent->control2.pt.getX(), prevNode->anchor.pt.getX()));
          Handle &ctrl1 = prevNode->control1;
          ctrl1.setX(juce::jmin(ctrl1.pt.getX(), selectedPoint.getX()));
          Handle &ctrl2 = prevNode->control2;
          ctrl2.setX(juce::jmin(ctrl2.pt.getX(), selectedPoint.getX()));
        }
        if (nextNode) {
          selectedPoint.setX(juce::jmin(selectedPoint.getX(), nextNode->anchor.pt.getX()));
          selectedHandle->parent->control1.pt.setX(
            juce::jmin(selectedHandle->parent->control1.pt.getX(), nextNode->anchor.pt.getX()));
          selectedHandle->parent->control2.pt.setX(
            juce::jmin(selectedHandle->parent->control2.pt.getX(), nextNode->anchor.pt.getX()));
          Handle &ctrl1 = nextNode->control1;
          ctrl1.setX(juce::jmax(ctrl1.pt.getX(), selectedPoint.getX()));
          Handle &ctrl2 = nextNode->control2;
          ctrl2.setX(juce::jmax(ctrl2.pt.getX(), selectedPoint.getX()));
        }
        selectedHandle->parent->setAnchorPt(selectedPoint);
      } else if (curveType == CurveType::Quadratic || curveType == CurveType::Cubic) {
        // Lock handles to be within the X coordinates of the anchor points
        T minX = selectedHandle->parent->anchor.pt.getX();
        selectedPoint.setX(juce::jmax(minX, selectedPoint.getX()));
        if (nextNode) {
          T maxX = nextNode->anchor.pt.getX();
          selectedPoint.setX(juce::jmin(maxX, selectedPoint.getX()));
        }

        if (selectedHandle == &selectedHandle->parent->control1) {
          selectedHandle->parent->setControlPt1(selectedPoint);
        } else if (selectedHandle == &selectedHandle->parent->control2 && curveType == CurveType::Cubic) {
          selectedHandle->parent->setControlPt2(selectedPoint);
        }
      }

      repaint();
    }
  }

  template<typename T>
  void CurveEditor<T>::mouseUp(const juce::MouseEvent &) {
    selectedHandle = nullptr;
    repaint();
  }

  template<typename T>
  void CurveEditor<T>::mouseDoubleClick(const juce::MouseEvent &event) {
    const PointType mousePt = event.mouseDownPosition;
    Handle *closestHandle = getClosestHandle(mousePt, DISTANCE_THRESHOLD);
    if (!closestHandle) {
      const PointType modelSpaceMousePt = transformPointFromScreenSpace(mousePt);
      addPoint(modelSpaceMousePt);
      return;
    }

    //const PointType& closestPt = closestHandle->pt;
    Node *closestNode = closestHandle->parent;

    //auto closestPointDist = transformPointToScreenSpace (closestPt).getDistanceFrom (mousePt);

    if (closestHandle == &closestNode->anchor) {
      auto newType = static_cast<CurveType> ((static_cast<int> (closestNode->curveType) + 1) % CurveEditorModel<
        T>::CurveTypeCount);
      closestNode->curveType = newType;

      if (newType == CurveType::Linear) {
        closestNode->setControlPt1(closestNode->anchor.pt);
      } else if (newType == CurveType::Quadratic && closestNode != model.nodes.back().get()) {
        constexpr int DEFAULT_CONTROL_DISTANCE = 5;
        PointType controlPoint1 = closestNode->anchor.pt + PointType(DEFAULT_CONTROL_DISTANCE, 0);
        closestHandle->parent->setControlPt1(controlPoint1);
      } else if (newType == CurveType::Cubic && closestNode != model.nodes.back().get()) {
        constexpr int DEFAULT_CONTROL_DISTANCE = 5;
        PointType controlPoint1 = closestNode->anchor.pt + PointType(0, DEFAULT_CONTROL_DISTANCE);
        PointType controlPoint2 = closestNode->anchor.pt + PointType(DEFAULT_CONTROL_DISTANCE, 0);
        closestHandle->parent->setControlPt1(controlPoint1);
        closestHandle->parent->setControlPt2(controlPoint2);
      }
    }
  }

  template<typename T>
  void CurveEditor<T>::mouseMove(const juce::MouseEvent &) {
    repaint();
  }

  template<typename T>
  void CurveEditor<T>::resized() {
    screenSpaceTransform = juce::AffineTransform();
    screenSpaceTransform = screenSpaceTransform.translated(-model.minX, -model.maxY);
    screenSpaceTransform = screenSpaceTransform.scaled(static_cast<float> (getWidth()) / (model.maxX - model.minX),
                                                       static_cast<float> (getHeight()) / (model.minY - model.maxY
                                                       ));
  }

  template<typename T>
  void CurveEditor<T>::valueChanged(juce::Value &) {
    repaint();
  }

  template<typename T>
  void CurveEditor<T>::addPoint(const PointType &p) {
    for (size_t i = 0; i < model.nodes.size(); i++) {
      const auto &point = *model.nodes[i];
      if (p.x <= point.anchor.pt.x) {
        model.nodes.emplace(model.nodes.begin() + static_cast<int>(i), std::make_shared<Node>(p));
        repaint();
        return;
      }
    }
  }

  template<typename T>
  typename CurveEditor<T>::Handle *CurveEditor<T>::getClosestHandle(const PointType &screenPt, T thresholdDistance) {
    jassert (!model.nodes.empty());

    auto modelPt = transformPointFromScreenSpace(screenPt);
    Handle *closestHandle = nullptr;
    float closestHandleDist = 0;
    for (auto &node: model.nodes) {
      auto dist = modelPt.getDistanceFrom(node->anchor.pt);
      if (dist < closestHandleDist || closestHandle == nullptr) {
        closestHandle = &node->anchor;
        closestHandleDist = dist;
      }

      if (node->curveType == CurveType::Quadratic) {
        dist = modelPt.getDistanceFrom(node->control1.pt);
        if (dist < closestHandleDist || closestHandle == nullptr) {
          closestHandle = &node->control1;
          closestHandleDist = dist;
        }
      }

      if (node->curveType == CurveType::Cubic) {
        dist = modelPt.getDistanceFrom(node->control1.pt);
        if (dist < closestHandleDist || closestHandle == nullptr) {
          closestHandle = &node->control1;
          closestHandleDist = dist;
        }
        dist = modelPt.getDistanceFrom(node->control2.pt);
        if (dist < closestHandleDist || closestHandle == nullptr) {
          closestHandle = &node->control2;
          closestHandleDist = dist;
        }
      }
    }

    if (thresholdDistance > 0 &&
        screenPt.getDistanceFrom(transformPointToScreenSpace(closestHandle->pt)) > thresholdDistance) {
      return nullptr;
    } else {
      return closestHandle;
    }
  }

  template<typename T>
  std::shared_ptr<typename CurveEditor<T>::Node>
  CurveEditor<T>::getNextNode(const typename CurveEditor<T>::Node &fromNode) {
    for (size_t i = 0; i < model.nodes.size() - 1; i++) {
      if (model.nodes[i].get() == &fromNode) {
        return model.nodes[i + 1];
      }
    }
    return nullptr;
  }

  template<typename T>
  std::shared_ptr<typename CurveEditor<T>::Node>
  CurveEditor<T>::getPrevNode(const typename CurveEditor<T>::Node &fromNode) {
    for (size_t i = 1; i < model.nodes.size(); i++) {
      if (model.nodes[i].get() == &fromNode) {
        return model.nodes[i - 1];
      }
    }
    return nullptr;
  }

  template<typename T>
  typename CurveEditor<T>::PointType CurveEditor<T>::transformPointToScreenSpace(const PointType &p) const {
    return p.transformedBy(screenSpaceTransform);
  }

  template<typename T>
  typename CurveEditor<T>::PointType CurveEditor<T>::transformPointFromScreenSpace(const PointType &p) const {
    return p.transformedBy(screenSpaceTransform.inverted());
  }
}