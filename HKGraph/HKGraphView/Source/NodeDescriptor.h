#pragma once
#include "Graph.h"
#include "GraphViewTheme.h"
#include "JuceHeader.h"
#include "NodeProcessor.h"

enum class NodeDescriptorKind {
  Plain = 0, Host = 1,
};

struct NodeDescriptor {
  NodeDescriptorKind kind;
  GraphViewTheme theme;
  NodeProcessor *processor{};
  bool selected{};
  unsigned int cNodeBackgroundCurrent{};
  float scaleFactor{};
  juce::Point<float> position;
  int width{};
  int height{};
  juce::AffineTransform translation;
  juce::AffineTransform scale;
  juce::ValueTree nodeData{"nd-node-data"};

  void save(juce::ValueTree& tree) const {

    if (kind == NodeDescriptorKind::Plain) {
      tree.setProperty("nd-kind", juce::var(0), nullptr);
    } else if (kind == NodeDescriptorKind::Host) {
      tree.setProperty("nd-kind", juce::var(1), nullptr);
    }

    tree.setProperty("nd-selected", juce::var(selected), nullptr);

    tree.setProperty("nd-cNodeBackgroundCurrent", juce::var(static_cast<int>(cNodeBackgroundCurrent)), nullptr);

    tree.setProperty("nd-scaleFactor", juce::var(scaleFactor), nullptr);

    tree.setProperty("nd-position-x", juce::var(position.x), nullptr);
    tree.setProperty("nd-position-y", juce::var(position.y), nullptr);

    tree.setProperty("nd-width", juce::var(width), nullptr);
    tree.setProperty("nd-height", juce::var(height), nullptr);

    tree.setProperty("nd-translation-x", juce::var(translation.getTranslationX()), nullptr);
    tree.setProperty("nd-translation-y", juce::var(translation.getTranslationY()), nullptr);

    tree.setProperty("nd-scale", juce::var(std::sqrt(std::abs(scale.getDeterminant()))), nullptr);
    tree.appendChild(nodeData, nullptr);
  }

  static NodeDescriptor *restore(const juce::ValueTree& tree) {
    auto nd = new NodeDescriptor;

    int kind = tree.getProperty("nd-kind");
    nd->kind = NodeDescriptorKind::Plain;
    if (kind == 0) {
      nd->kind = NodeDescriptorKind::Plain;
    } else if (kind == 1) {
      nd->kind = NodeDescriptorKind::Host;
    }

    nd->selected = tree.getProperty("nd-selected");

    int cNodeBackgroundCurrent = tree.getProperty("nd-cNodeBackgroundCurrent");
    nd->cNodeBackgroundCurrent = static_cast<unsigned int>(cNodeBackgroundCurrent);

    nd->scaleFactor = tree.getProperty("nd-scaleFactor");

    float positionX = tree.getProperty("nd-position-x");
    float positionY = tree.getProperty("nd-position-y");
    nd->position = juce::Point<float>(positionX, positionY);

    nd->width = tree.getProperty("nd-width");
    nd->height = tree.getProperty("nd-height");

    float translationX = tree.getProperty("nd-translation-x");
    float translationY = tree.getProperty("nd-translation-y");
    nd->translation = juce::AffineTransform::translation(translationX, translationY);

    float scale = tree.getProperty("nd-scale");
    nd->scale = juce::AffineTransform::scale(scale);

    nd->nodeData = tree.getChildWithName("nd-node-data");
    return nd;
  }
};
