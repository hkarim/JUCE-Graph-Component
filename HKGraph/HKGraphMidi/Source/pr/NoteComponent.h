#pragma once

#include "JuceHeader.h"
#include "NoteModel.h"
#include "PianoRollTheme.h"

struct NoteComponent : juce::Component {
  bool selected{false};
  bool dragging{false};
  const juce::Colour cSelectedBg{PianoRollTheme::noteSelectedBg};
  const juce::Colour cBorderFg{juce::Colours::whitesmoke};
  const float borderWidth{PianoRollTheme::noteBorderWidth};
  juce::Point<int> dragMouseDownPosition{};
  NoteModel model{};
  std::function<void(NoteComponent *, bool)> callback;

  explicit NoteComponent(juce::ComponentBoundsConstrainer *constrainer) :
    juce::Component(),
    rightResizable(this, constrainer, juce::ResizableEdgeComponent::Edge::rightEdge),
    leftResizable(this, constrainer, juce::ResizableEdgeComponent::Edge::leftEdge) {
    addAndMakeVisible(rightResizable);
    addAndMakeVisible(leftResizable);
  }

  ~NoteComponent() override = default;

  void paint(juce::Graphics &g) override {
    auto bounds = getLocalBounds();

    auto velocity = static_cast<int>(juce::jmap(model.velocity, 1.0f, 100.0f));
    auto darkness = static_cast<float>((velocity % 10) * 0.01);
    auto cUnselected = juce::Colours::white;
    if (0 <= velocity && velocity <= 10)
      cUnselected = juce::Colour(0xFF43426C).darker(darkness);
    else if (10 <= velocity && velocity <= 20)
      cUnselected = juce::Colour(0xFF557DC2).darker(darkness);
    else if (20 <= velocity && velocity <= 30)
      cUnselected = juce::Colour(0xFF46A0B4).darker(darkness);
    else if (30 <= velocity && velocity <= 40)
      cUnselected = juce::Colour(0xFF42B187).darker(darkness);
    else if (40 <= velocity && velocity <= 50)
      cUnselected = juce::Colour(0xFF46B446).darker(darkness);
    else if (50 <= velocity && velocity <= 60)
      cUnselected = juce::Colour(0xFF80AA3C).darker(darkness);
    else if (60 <= velocity && velocity <= 70)
      cUnselected = juce::Colour(0xFFAA963C).darker(darkness);
    else if (70 <= velocity && velocity <= 80)
      cUnselected = juce::Colour(0xFFB8744A).darker(darkness);
    else if (80 <= velocity && velocity <= 90)
      cUnselected = juce::Colour(0xFFC15D53).darker(darkness);
    else if (90 <= velocity && velocity <= 100)
      cUnselected = juce::Colour(0xFFC45757).darker(darkness);

    auto borderThickness = 1;
    // body
    if (selected) {
      g.setColour(cSelectedBg);
      g.fillRect(bounds);
    } else {
      g.setColour(cUnselected);
      g.fillRect(bounds);
      auto velocityWidth = juce::jmap(model.velocity, 0.0f, static_cast<float>(bounds.getWidth()));
      auto vw = static_cast<int>(velocityWidth);
      // draw remaining
      g.setColour(cUnselected.brighter());
      auto remaining = juce::Rectangle<int>(
        vw,
        borderThickness,
        bounds.getWidth() - vw - borderThickness,
        bounds.getHeight() - (2 * borderThickness));
      g.fillRect(remaining);
    }

    // text
    auto h = static_cast<float>(bounds.getHeight()) * model.scaledHeight;
    auto fh = g.getCurrentFont().getHeight();
    if (h >= fh) {
      auto n = 127 - model.lane;
      auto at = juce::AffineTransform().scaled(1.0f / model.scaledWidth, 1.0f / model.scaledHeight);
      auto delta = std::abs(h - fh);
      auto spacing = delta / 2;
      if (selected) { // font colour is the reverse of the background colour
        g.setColour(cUnselected);
      } else {
        g.setColour(cSelectedBg);
      }
      g.addTransform(at); // prevent font transformation

      auto textArea = juce::Rectangle<float>(
        4.0f / model.scaledWidth,
        spacing / model.scaledHeight,
        static_cast<float>(bounds.getWidth()) * model.scaledWidth,
        static_cast<float>(bounds.getHeight()) * model.scaledHeight
      );
      g.drawText(
        juce::MidiMessage::getMidiNoteName(n, true, true, 3),
        textArea,
        juce::Justification::centredLeft,
        false);
    }

  }

  void resized() override {
    auto bounds = getLocalBounds();
    rightResizable.setBounds(bounds.getWidth() - 2, 0, 2, getHeight());
    leftResizable.setBounds(0, 0, 2, getHeight());
  }

  struct ResizableNote : juce::ResizableEdgeComponent {
    NoteComponent *note;

    ResizableNote(NoteComponent *componentToResize,
                  juce::ComponentBoundsConstrainer *constrainer,
                  juce::ResizableEdgeComponent::Edge edgeToResize) :
      juce::ResizableEdgeComponent(componentToResize, constrainer, edgeToResize),
      note(componentToResize) {
    }

    ~ResizableNote() override = default;

    void mouseDown(const juce::MouseEvent &e) override {
      if (note->callback != nullptr) {
        note->callback(note, e.mods.isShiftDown());
      }
      juce::ResizableEdgeComponent::mouseDown(e);
    }

    void mouseUp(const juce::MouseEvent &e) override {
      if (note->callback != nullptr) {
        note->callback(note, false);
      }
      juce::ResizableEdgeComponent::mouseUp(e);
    }

  };

  void mouseWheelMove(const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel) override {
    if (event.mods.isShiftDown()) {
      auto reversed = wheel.isReversed ? -1.0f : 1.0f;
      auto delta = juce::jmap(reversed * wheel.deltaX, 0.0f, 1.0f); // weird
      model.velocity += -1 * reversed * delta;
      if (model.velocity < 0.0f)
        model.velocity = 0.0f;
      else if (model.velocity > 1.0f)
        model.velocity = 1.0f;
      repaint();
    }
  }

private:
  ResizableNote rightResizable;
  ResizableNote leftResizable;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NoteComponent)
};