#include "SelectionComponent.h"

SelectionComponent::SelectionComponent(unsigned int backgroundColor) {
  setOpaque(false);
  setAlwaysOnTop(true);
  cSelectionBackground = backgroundColor;
}

SelectionComponent::~SelectionComponent() = default;


void SelectionComponent::paint(juce::Graphics &g) {
  auto w = static_cast<float>(getWidth());
  auto h = static_cast<float>(getHeight());

  juce::Path p;
  p.addRectangle(0, 0, w, h);
  g.setColour(juce::Colour(cSelectionBackground));
  g.fillPath(p);
}


void SelectionComponent::calculateBounds(const juce::Point<int> &start, const juce::Point<int> &offset) {

  auto w = abs(offset.x);
  auto h = abs(offset.y);
  auto up = offset.y < 0;
  auto down = offset.y >= 0;
  auto left = offset.x < 0;
  auto right = offset.x >= 0;

  w = (w > 1) ? w : 2;
  h = (h > 1) ? h : 2;


  if (down && right) {
    setBounds(start.x, start.y, w, h);
  } else if (up && left) {
    setBounds(start.x - w, start.y - h, w, h);
  } else if (up) {
    setBounds(start.x, start.y - h, w, h);
  } else {
    setBounds(start.x - w, start.y, w, h);
  }
}
