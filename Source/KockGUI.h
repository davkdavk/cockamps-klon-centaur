#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "BinaryData.h"

// Draws the knobs in the photo's maroon colour. The body is fixed; the pointer
// tab rotates with the slider value.
class PointerOnly : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h,
                           float pos, float s0, float s1, juce::Slider&) override
    {
        const auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (2.0f);
        const auto c = bounds.getCentre();
        const float r = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
        const float body = r * 0.9f;
        const float angle = s0 + pos * (s1 - s0);
        const auto brown = juce::Colour::fromRGB (85, 58, 55);

        // shadow and raised outer rim
        g.setColour (juce::Colours::black.withAlpha (0.35f));
        g.fillEllipse (c.x - body + 3.0f, c.y - body + 4.0f, body * 2.0f, body * 2.0f);
        g.setColour (brown.darker (0.65f));
        g.fillEllipse (c.x - body, c.y - body, body * 2.0f, body * 2.0f);

        // body
        juce::ColourGradient grad (brown.brighter (0.25f), c.x - body * 0.4f, c.y - body * 0.5f,
                                   brown.darker (0.35f), c.x + body, c.y + body, true);
        g.setGradientFill (grad);
        g.fillEllipse (c.x - body + 4.0f, c.y - body + 4.0f, body * 2.0f - 8.0f, body * 2.0f - 8.0f);

        g.setColour (brown.darker (0.5f));
        g.drawEllipse (c.x - body + 4.0f, c.y - body + 4.0f, body * 2.0f - 8.0f, body * 2.0f - 8.0f, 2.0f);

        // Raised moulded index rotates with the knob face.
        juce::Path tab;
        tab.addRoundedRectangle (-5.5f, -body * 0.98f, 11.0f, body * 0.32f, 4.0f);
        tab.applyTransform (juce::AffineTransform::rotation (angle).translated (c.x, c.y));
        g.setColour (brown.brighter (0.38f));
        g.fillPath (tab);
        g.setColour (brown.darker (0.25f));
        g.strokePath (tab, juce::PathStrokeType (1.2f));
    }
};

class CentaurFootswitch : public juce::Button
{
public:
    CentaurFootswitch() : juce::Button ("Effect on/off") { setClickingTogglesState (true); }

    void paintButton (juce::Graphics& g, bool, bool) override
    {
        const auto area = getLocalBounds().toFloat();
        const auto c = area.getCentre();
        const float r = juce::jmin (area.getWidth(), area.getHeight()) * 0.38f;
        const float shift = getToggleState() ? 0.0f : 3.0f;

        const auto button = juce::Rectangle<float> (c.x - r, c.y - r + shift + 4.0f, r * 2.0f, r * 2.0f);
        juce::ColourGradient metal (juce::Colour::fromRGB (247, 235, 202), button.getX(), button.getY(),
                                    juce::Colour::fromRGB (116, 100, 76), button.getRight(), button.getBottom(), true);
        g.setGradientFill (metal);
        g.fillEllipse (button);
        g.setColour (juce::Colour::fromRGB (75, 58, 44));
        g.drawEllipse (button, 2.0f);
        g.setColour (juce::Colour::fromRGB (255, 249, 224).withAlpha (0.9f));
        g.fillEllipse (button.reduced (r * 0.28f));
    }
};

class KockGUI : public juce::AudioProcessorEditor
{
public:
    explicit KockGUI (juce::AudioProcessor& p) : juce::AudioProcessorEditor (&p), proc (p)
    {
        img = juce::ImageCache::getFromMemory (BinaryData::klon_png,
                                               BinaryData::klon_pngSize);
        setLookAndFeel (&lnf);

        int idx = 0;
        for (auto* s : { &gainS, &trebleS, &outputS })
        {
            s->setSliderStyle (juce::Slider::RotaryVerticalDrag);
            s->setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
            s->setRotaryParameters (juce::MathConstants<float>::pi * 1.25f,
                                    juce::MathConstants<float>::pi * 2.75f, true);
            s->setRange (0.0, 1.0);
            addAndMakeVisible (s);

            if (auto* prm = dynamic_cast<juce::RangedAudioParameter*> (proc.getParameters()[idx]))
            {
                auto a = std::make_unique<juce::ParameterAttachment> (*prm,
                    [s] (float v) { s->setValue (v, juce::dontSendNotification); });
                a->sendInitialUpdate();
                s->onValueChange = [s, raw = a.get()] { raw->setValueAsCompleteGesture ((float) s->getValue()); };
                atts.push_back (std::move (a));
            }
            ++idx;
        }

        addAndMakeVisible (footswitch);
        if (proc.getParameters().size() > 3)
        {
            if (auto* prm = dynamic_cast<juce::RangedAudioParameter*> (proc.getParameters()[3]))
            {
                enabledAttachment = std::make_unique<juce::ParameterAttachment> (*prm,
                    [this] (float v)
                    {
                        effectEnabled = v >= 0.5f;
                        footswitch.setToggleState (effectEnabled, juce::dontSendNotification);
                        repaint();
                    });
                enabledAttachment->sendInitialUpdate();
                footswitch.onClick = [this]
                {
                    if (enabledAttachment != nullptr)
                    {
                        effectEnabled = footswitch.getToggleState();
                        enabledAttachment->setValueAsCompleteGesture (effectEnabled ? 1.0f : 0.0f);
                        repaint();
                    }
                };
            }
        }

        setSize (juce::roundToInt (img.getWidth() * uiScale),
                 juce::roundToInt (img.getHeight() * uiScale));
    }

    ~KockGUI() override { enabledAttachment.reset(); atts.clear(); setLookAndFeel (nullptr); }

    void paint (juce::Graphics& g) override
    {
        g.drawImage (img, getLocalBounds().toFloat());

        // Light the photographed LED globe beside the GAIN label.
        const juce::Point<float> ledCentre (67.0f * uiScale, 229.0f * uiScale);
        const float ledRadius = 9.0f * uiScale;
        if (effectEnabled)
        {
            g.setColour (juce::Colours::red.withAlpha (0.24f));
            g.fillEllipse (ledCentre.x - 14.0f * uiScale, ledCentre.y - 14.0f * uiScale,
                           28.0f * uiScale, 28.0f * uiScale);
        }

        g.setColour (juce::Colour::fromRGB (67, 35, 31));
        g.fillEllipse (ledCentre.x - ledRadius - 1.5f * uiScale, ledCentre.y - ledRadius - 1.5f * uiScale,
                       (ledRadius + 1.5f * uiScale) * 2.0f, (ledRadius + 1.5f * uiScale) * 2.0f);
        juce::ColourGradient led (effectEnabled ? juce::Colour::fromRGB (255, 84, 59)
                                                 : juce::Colour::fromRGB (95, 36, 29),
                                  ledCentre.x - 3.0f * uiScale, ledCentre.y - 4.0f * uiScale,
                                  effectEnabled ? juce::Colour::fromRGB (153, 12, 9)
                                                : juce::Colour::fromRGB (53, 22, 18),
                                  ledCentre.x + 4.0f * uiScale, ledCentre.y + 5.0f * uiScale, true);
        g.setGradientFill (led);
        g.fillEllipse (ledCentre.x - ledRadius, ledCentre.y - ledRadius,
                       ledRadius * 2.0f, ledRadius * 2.0f);
        if (effectEnabled)
        {
            g.setColour (juce::Colours::white.withAlpha (0.72f));
            g.fillEllipse (ledCentre.x - 4.0f * uiScale, ledCentre.y - 5.0f * uiScale,
                           4.0f * uiScale, 3.0f * uiScale);
        }
    }

    void resized() override
    {
        // Knob centres measured in the 786x612 artwork
        const int diameter = juce::roundToInt (170.0f * uiScale);
        auto place = [diameter] (juce::Component& c, int cx, int cy)
        {
            c.setBounds (cx - diameter / 2, cy - diameter / 2, diameter, diameter);
        };
        place (gainS,   juce::roundToInt (148.0f * uiScale), juce::roundToInt (130.0f * uiScale));
        place (trebleS, juce::roundToInt (393.0f * uiScale), juce::roundToInt (120.0f * uiScale));
        place (outputS, juce::roundToInt (640.0f * uiScale), juce::roundToInt (115.0f * uiScale));
        footswitch.setBounds (juce::roundToInt (480.0f * uiScale), juce::roundToInt (431.0f * uiScale),
                              juce::roundToInt (76.0f * uiScale), juce::roundToInt (76.0f * uiScale));
    }

private:
    static constexpr float uiScale = 0.6f;

    juce::AudioProcessor& proc;
    juce::Image img;
    PointerOnly lnf;
    juce::Slider gainS, trebleS, outputS;
    CentaurFootswitch footswitch;
    bool effectEnabled = true;
    std::vector<std::unique_ptr<juce::ParameterAttachment>> atts;
    std::unique_ptr<juce::ParameterAttachment> enabledAttachment;
};
