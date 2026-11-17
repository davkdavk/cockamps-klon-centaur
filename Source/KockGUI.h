#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "BinaryData.h"

// Every control position lives here, in the coordinates of the 786x612 artwork, so the knobs,
// their labels and the printed artwork text can't drift apart.
//  - The three big knobs share one centre line and sit exactly over the printed
//    GAIN / TREBLE / OUTPUT labels (centres measured from the artwork).
//  - The two small mid knobs sit on the printed-label row, centred in the gaps between the words.
//  - The DIODE switch sits in the OUTPUT column.
namespace Layout
{
    constexpr float scale = 0.6f;                 // artwork -> editor pixels

    constexpr float bigDiameter = 170.0f;
    constexpr float bigRowY     = 122.0f;
    constexpr float gainX       = 144.0f;         // printed label centres
    constexpr float trebleX     = 389.0f;
    constexpr float outputX     = 637.0f;

    constexpr float labelRowY   = 228.0f;         // vertical centre of the printed labels
    constexpr float midDiameter = 93.0f;
    constexpr float midFreqX    = 259.0f;         // centred between GAIN and TREBLE
    constexpr float midDepthX   = 511.5f;         // centred between TREBLE and OUTPUT
    constexpr float midFreqLabelShiftX = 10.0f;    // artwork pixels
    constexpr float midDepthLabelShiftX = 5.0f;    // artwork pixels; half the Mid Freq nudge

    constexpr float diodeDiameter = 57.0f;
    constexpr float diodeX      = outputX;
    constexpr float diodeY      = 388.0f;
}

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
        const float tabWidth = body * 0.25f; // scales with the knob so small knobs don't get an oversized tab
        tab.addRoundedRectangle (-tabWidth * 0.5f, -body * 0.98f, tabWidth, body * 0.32f, body * 0.09f);
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

class CentaurDiodeButton : public juce::Button
{
public:
    CentaurDiodeButton() : juce::Button ("Clipping diode type") { setClickingTogglesState (true); }

    void paintButton (juce::Graphics& g, bool, bool) override
    {
        const auto bounds = getLocalBounds().toFloat();
        const auto centre = bounds.getCentre();
        const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f - 2.0f;
        const float press = getToggleState() ? 2.0f : 0.0f;

        g.setColour (juce::Colours::black.withAlpha (0.35f));
        g.fillEllipse (centre.x - radius + 1.0f, centre.y - radius + 2.0f, radius * 2.0f, radius * 2.0f);
        g.setColour (juce::Colour::fromRGB (74, 49, 34));
        g.fillEllipse (centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);
        g.setColour (juce::Colour::fromRGB (205, 169, 106));
        g.drawEllipse (centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f, 1.5f);

        const auto face = juce::Rectangle<float> (centre.x - radius + 4.0f,
                                                   centre.y - radius + 4.0f + press,
                                                   (radius - 4.0f) * 2.0f, (radius - 4.0f) * 2.0f);
        juce::ColourGradient metal (getToggleState() ? juce::Colour::fromRGB (177, 126, 59)
                                                     : juce::Colour::fromRGB (244, 218, 157),
                                    face.getX(), face.getY(), juce::Colour::fromRGB (127, 81, 43),
                                    face.getRight(), face.getBottom(), true);
        g.setGradientFill (metal);
        g.fillEllipse (face);
        g.setColour (juce::Colour::fromRGB (65, 39, 26));
        g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
        g.drawText (getToggleState() ? "S" : "G", face, juce::Justification::centred);
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

        configureMidSlider (midFreqSlider, 250.0, 2500.0, 1.0);
        midFreqSlider.setName ("Mid Freq");
        midFreqSlider.setSkewFactorFromMidPoint (800.0);
        attachSlider (midFreqSlider, 5);

        configureMidSlider (midDepthSlider, -9.0, 6.0, 0.1);
        midDepthSlider.setName ("Mid Depth");
        attachSlider (midDepthSlider, 6);

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

        addAndMakeVisible (diodeSwitch);
        if (proc.getParameters().size() > 4)
        {
            if (auto* prm = dynamic_cast<juce::RangedAudioParameter*> (proc.getParameters()[4]))
            {
                diodeAttachment = std::make_unique<juce::ParameterAttachment> (*prm,
                    [this] (float value) { diodeSwitch.setToggleState (value >= 0.5f, juce::dontSendNotification); });
                diodeAttachment->sendInitialUpdate();
                diodeSwitch.onClick = [this]
                {
                    if (diodeAttachment != nullptr)
                        diodeAttachment->setValueAsCompleteGesture (diodeSwitch.getToggleState() ? 1.0f : 0.0f);
                };
            }
        }

        setSize (juce::roundToInt (img.getWidth() * uiScale), scaledImageHeight);
    }

    ~KockGUI() override { diodeAttachment.reset(); enabledAttachment.reset(); atts.clear(); setLookAndFeel (nullptr); }

    void paint (juce::Graphics& g) override
    {
        g.drawImage (img, juce::Rectangle<float> (0.0f, 0.0f, getWidth(), (float) scaledImageHeight));

        // Text labels sit directly on the pedal fascia, centred on their control; no backing panel.
        // The mid labels go above their knobs (below them is the centaur and the title lettering).
        g.setFont (juce::FontOptions (10.5f, juce::Font::bold).withName (juce::Font::getDefaultSerifFontName()).withKerningFactor (0.08f));
        g.setColour (juce::Colour::fromRGB (96, 46, 30));
        auto drawLabel = [&g] (const char* text, float artX, float artY)
        {
            g.drawText (text, juce::Rectangle<float> (80.0f, 14.0f).withCentre ({ artX * Layout::scale, artY * Layout::scale }),
                        juce::Justification::centred);
        };
        const float midLabelY = Layout::labelRowY - Layout::midDiameter * 0.5f - 9.0f;
        drawLabel ("MID FREQ", Layout::midFreqX + Layout::midFreqLabelShiftX, midLabelY);
        drawLabel ("MID DEPTH", Layout::midDepthX + Layout::midDepthLabelShiftX, midLabelY);
        drawLabel ("DIODE", Layout::diodeX, Layout::diodeY + Layout::diodeDiameter * 0.5f + 14.0f);

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
        // Square components centred on a point in artwork coordinates (all positions come from Layout)
        auto place = [] (juce::Component& c, float artX, float artY, float artSize)
        {
            const int size = juce::roundToInt (artSize * Layout::scale);
            c.setBounds (juce::roundToInt (artX * Layout::scale) - size / 2,
                         juce::roundToInt (artY * Layout::scale) - size / 2, size, size);
        };
        place (gainS,   Layout::gainX,   Layout::bigRowY, Layout::bigDiameter);
        place (trebleS, Layout::trebleX, Layout::bigRowY, Layout::bigDiameter);
        place (outputS, Layout::outputX, Layout::bigRowY, Layout::bigDiameter);
        place (midFreqSlider,  Layout::midFreqX,  Layout::labelRowY, Layout::midDiameter);
        place (midDepthSlider, Layout::midDepthX, Layout::labelRowY, Layout::midDiameter);
        place (diodeSwitch, Layout::diodeX, Layout::diodeY, Layout::diodeDiameter);
        footswitch.setBounds (juce::roundToInt (480.0f * uiScale), juce::roundToInt (431.0f * uiScale),
                              juce::roundToInt (76.0f * uiScale), juce::roundToInt (76.0f * uiScale));
    }

private:
    static constexpr float uiScale = Layout::scale;
    static constexpr int scaledImageHeight = 367;

    void configureMidSlider (juce::Slider& slider, double minimum, double maximum, double interval)
    {
        slider.setSliderStyle (juce::Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f,
                                    juce::MathConstants<float>::pi * 2.75f, true);
        slider.setRange (minimum, maximum, interval);
        addAndMakeVisible (slider);
    }

    void attachSlider (juce::Slider& slider, int parameterIndex)
    {
        if (parameterIndex < proc.getParameters().size())
        {
            if (auto* parameter = dynamic_cast<juce::RangedAudioParameter*> (proc.getParameters()[parameterIndex]))
            {
                auto attachment = std::make_unique<juce::ParameterAttachment> (*parameter,
                    [&slider] (float value) { slider.setValue (value, juce::dontSendNotification); });
                attachment->sendInitialUpdate();
                slider.onValueChange = [&slider, raw = attachment.get()]
                {
                    raw->setValueAsCompleteGesture ((float) slider.getValue());
                };
                atts.push_back (std::move (attachment));
            }
        }
    }

    juce::AudioProcessor& proc;
    juce::Image img;
    PointerOnly lnf;
    juce::Slider gainS, trebleS, outputS;
    juce::Slider midFreqSlider, midDepthSlider;
    CentaurFootswitch footswitch;
    CentaurDiodeButton diodeSwitch;
    bool effectEnabled = true;
    std::vector<std::unique_ptr<juce::ParameterAttachment>> atts;
    std::unique_ptr<juce::ParameterAttachment> enabledAttachment;
    std::unique_ptr<juce::ParameterAttachment> diodeAttachment;
};
