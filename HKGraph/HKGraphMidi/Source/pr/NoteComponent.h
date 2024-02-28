#pragma once
#include "JuceHeader.h"
#include "NoteModel.h"
#include "PianoRollTheme.h"

struct NoteComponent : juce::Component {
  bool selected{false};
  bool dragging{false};
  const juce::Colour cSelectedBg{PianoRollTheme::noteSelectedBg};
  const juce::Colour cUnselectedBg{PianoRollTheme::noteUnselectedBg};
  const juce::Colour cBorderFg{juce::Colours::whitesmoke};
  const float borderWidth{PianoRollTheme::noteBorderWidth};
  juce::Point<int> dragMouseDownPosition{};
  NoteModel model{};
  std::function<void(NoteComponent*, bool)> callback;

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
    bounds = bounds.reduced(1);

    // body
    if (selected) {
      g.setColour(cSelectedBg);
    } else {
      g.setColour(cUnselectedBg);
    }
    g.fillRect(bounds);

    // text
    auto h = static_cast<float>(bounds.getHeight()) * model.scaledHeight;
    auto fh = g.getCurrentFont().getHeight();
    if (h >= fh) {
      auto n = 127 - model.lane;
      auto at = juce::AffineTransform().scaled(1.0f / model.scaledWidth, 1.0f / model.scaledHeight);
      auto delta = std::abs(h - fh);
      auto spacing = delta / 2;
      if (selected) { // font colour is the reverse of the background colour
        g.setColour(cUnselectedBg);
      } else {
        g.setColour(cSelectedBg);
      }
      g.addTransform(at); // prevent font transformation

      auto textArea = juce::Rectangle<float>(
        4.0f * model.scaledWidth,
        spacing,
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
      juce::ResizableEdgeComponent(componentToResize, constrainer, edgeToResize) ,
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


private:
  ResizableNote rightResizable;
  ResizableNote leftResizable;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NoteComponent)
};