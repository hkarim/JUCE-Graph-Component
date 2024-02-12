#include "UnboundEdgeComponent.h"

UnboundEdgeComponent::UnboundEdgeComponent() {
  startPin = nullptr;
  endPin = nullptr;
}

void UnboundEdgeComponent::paint(juce::Graphics &g) {
  auto w = static_cast<float>(getWidth());
  auto h = static_cast<float>(getHeight());

  juce::Point<float> pstart, pend;

  if (inverted) {
    pstart = juce::Point<float>(w, 0);
    pend = juce::Point<float>(0, h);
  } else {
    pstart = juce::Point<float>(0, 0);
    pend = juce::Point<float>(w, h);
  }

  juce::Path path;

  path.startNewSubPath(pstart.x, pstart.y);
  if (inverted) {
    path.cubicTo(w, h * 0.5f, 0.0f, h * 0.5f, pend.x, pend.y);
  } else {
    path.cubicTo(0.0f, h * 0.5f, w, h * 0.5f, pend.x, pend.y);
  }
  path.setUsingNonZeroWinding(true);
  juce::PathStrokeType ps(1.0f);
  float ls[] = {2.0f, 3.0f};
  ps.createDashedStroke(path, path, ls, 2);
  g.setColour(juce::Colour(190, 190, 190));
  g.strokePath(path, juce::PathStrokeType(1.0f));
}

void UnboundEdgeComponent::calculateBounds(const juce::Point<int> &start, const juce::Point<int> &end) {
  auto offset = end - start;
  auto w = abs(offset.x);
  auto h = abs(offset.y);
  auto up = offset.y < 0;
  auto down = offset.y >= 0;
  auto left = offset.x < 0;
  auto right = offset.x >= 0;

  w = (w > 1) ? w : 2;
  h = (h > 1) ? h : 2;

  inverted = (up && right) || (down && left);

  if (down && right) {
    setBounds(start.x, start.y, w, h);
  } else if (up && left) {
    setBounds(start.x - w, start.y - h, w, h);
  } else if (up && right) {
    setBounds(start.x, start.y - h, w, h);
  } else if (down && left) {
    setBounds(start.x - w, start.y, w, h);
  }
}
