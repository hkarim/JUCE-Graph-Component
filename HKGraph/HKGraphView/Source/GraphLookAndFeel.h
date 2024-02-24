#pragma once

#include "JuceHeader.h"

// modified from https://github.com/kushview/element [GPL]
// https://github.com/kushview/element/blob/develop/src/ui/style_v1.cpp

struct GraphLookAndFeel : public juce::LookAndFeel_V4 {

  int getSliderThumbRadius(juce::Slider &slider) override {
    return juce::jmin(12, slider.isHorizontal()
                          ? static_cast<int> ((float) slider.getHeight() * 0.77f)
                          : static_cast<int> ((float) slider.getWidth() * 0.77f));
  }


  void drawRotarySlider(juce::Graphics &g, int x, int y, int width, int height,
                        float sliderPos, const float rotaryStartAngle,
                        const float rotaryEndAngle, juce::Slider &slider) override {

    Style::drawDial(g, x, y, width, height, sliderPos, 0.f,
                    rotaryStartAngle, rotaryEndAngle, slider);
  }

  struct Style {
    static void drawDial(juce::Graphics &g, int x, int y, int width, int height,
                         float sliderPos, const float anchorPos,
                         const float rotaryStartAngle, const float rotaryEndAngle,
                         juce::Slider &slider) {
      const float radius = static_cast<float>(juce::jmin(width / 2, height / 2)) - 2.0f;
      const float centreX = static_cast<float>(x) + static_cast<float>(width) * 0.5f;
      const float centreY = static_cast<float>(y) + static_cast<float>(height) * 0.5f;
      const float rx = centreX - radius;
      const float ry = centreY - radius;
      const float rw = radius * 2.0f;

      const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
      const float anchor = rotaryStartAngle + anchorPos * (rotaryEndAngle - rotaryStartAngle);
      const float a1 = angle < anchor ? angle : anchor;
      const float a2 = angle < anchor ? anchor : angle;

      const bool isMouseOver = slider.isMouseOverOrDragging() && slider.isEnabled();

      if (radius > 12.0f) {
        int lineSize = (int) std::max(2.f, std::min(20.f, radius * 0.085f));
        float lineTrim = radius > 32.f ? -3.0f : -2.f;
        float lineOffset = radius > 32.f ? -4.f : -1.f;
        const float thickness = 0.82f;
        const float csf = rw - (rw * thickness);
        {
          const float delta = rw - (rw * thickness);
          juce::Path filledCircle;
          juce::ColourGradient grad(juce::Colours::whitesmoke.darker(0.f), rx, ry,
                                    juce::Colours::whitesmoke.darker(2.f), rx + rw, ry + rw,
                                    true);
          //g.setFillType(grad);
          g.setFillType(juce::Colours::whitesmoke.darker(2.f));

          filledCircle.addEllipse(juce::Rectangle<float>(rx, ry, rw, rw).reduced(delta));
          g.fillPath(filledCircle);
          g.setColour(juce::Colours::black.brighter(0.17f));

          g.strokePath(filledCircle, juce::PathStrokeType(1.23f));
        }

        if (slider.isEnabled())
          g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId).withAlpha(isMouseOver ? 1.0f : 0.88f));
        else
          g.setColour(juce::Colour(0x80808080));

        {
          juce::Path filledArc;
          filledArc.addPieSegment(rx, ry, rw, rw, a1, a2, thickness);
          g.fillPath(filledArc);
        }

        {
          juce::Path p;
          p.addLineSegment(juce::Line<float>(
            0.f,
            lineOffset,
            0.f,
            -radius + csf + std::abs(lineTrim)), static_cast<float>(lineSize));
          g.setColour(juce::Colours::black);
          g.fillPath(p, juce::AffineTransform::rotation(angle).translated(centreX, centreY));
        }

        if (slider.isEnabled())
          g.setColour(slider.findColour(juce::Slider::rotarySliderOutlineColourId));
        else
          g.setColour(juce::Colour(0x80808080));
      } else {
        if (slider.isEnabled())
          g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId).withAlpha(isMouseOver ? 1.0f : 0.7f));
        else
          g.setColour(juce::Colour(0x80808080));

        juce::Path p;
        p.addEllipse(-0.4f * rw, -0.4f * rw, rw * 0.8f, rw * 0.8f);
        juce::PathStrokeType(rw * 0.1f).createStrokedPath(p, p);

        p.addLineSegment(juce::Line<float>(0.0f, 0.0f, 0.0f, -radius), rw * 0.2f);

        g.fillPath(p, juce::AffineTransform::rotation(angle).translated(centreX, centreY));
      }
    }
  };

};