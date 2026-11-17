/* ==============================================================================
   Klone Centaur-style preamp: JUCE audio processor wrapper
   ============================================================================== */

#include <juce_audio_processors/juce_audio_processors.h>
#include "engine/KlonCentaurEngine.h"
#include "KockGUI.h"

class KlonePrepampProcessor final : public juce::AudioProcessor
{
public:
    KlonePrepampProcessor()
        : AudioProcessor (BusesProperties()
                              .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                              .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
          apvts_ (*this, nullptr, "PARAMETERS", createParameterLayout())
    {
        driveParam_ = apvts_.getRawParameterValue ("drive");
        toneParam_  = apvts_.getRawParameterValue ("tone");
        levelParam_ = apvts_.getRawParameterValue ("level");
        enabledParam_ = apvts_.getRawParameterValue ("enabled");
        diodeParam_ = apvts_.getRawParameterValue ("diode");
        midFreqParam_ = apvts_.getRawParameterValue ("midfreq");
        midDepthParam_ = apvts_.getRawParameterValue ("middepth");
    }

    //==============================================================================
    const juce::String getName() const override { return "CockAmps KOCK Centaur"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return "Default"; }
    void changeProgramName (int, const juce::String&) override {}

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override
    {
        engine_.prepare (sampleRate, samplesPerBlock);
        pushParameters();
    }

    void releaseResources() override {}

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override
    {
        return layouts.getMainInputChannelSet()  == juce::AudioChannelSet::stereo()
            && layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
    }

    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) override
    {
        juce::ScopedNoDenormals noDenormals;
        pushParameters();

        if (enabledParam_->load() < 0.5f)
            return;

        if (buffer.getNumChannels() != 2)
            return;

        engine_.processBlock (buffer);
    }

    //==============================================================================
    bool hasEditor() const override { return true; }

    juce::AudioProcessorEditor* createEditor() override
    {
        return new KockGUI (*this);
    }

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override
    {
        auto state = apvts_.copyState();
        std::unique_ptr<juce::XmlElement> xml (state.createXml());
        copyXmlToBinary (*xml, destData);
    }

    void setStateInformation (const void* data, int sizeInBytes) override
    {
        std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
        if (xml != nullptr && xml->hasTagName (apvts_.state.getType()))
            apvts_.replaceState (juce::ValueTree::fromXml (*xml));
    }

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
    {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;
        const juce::NormalisableRange<float> range (0.0f, 1.0f, 0.001f);

        layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "drive", 1 }, "Drive", range, 0.5f));
        layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "tone",  1 }, "Tone",  range, 0.5f));
        layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "level", 1 }, "Level", range, 0.5f));
        layout.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "enabled", 1 }, "Effect On", true));
        // Appended last so the existing parameter indices (0-3) and saved sessions are unaffected.
        layout.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "diode", 1 }, "Diode",
                                                                  juce::StringArray { "Germanium", "Silicon" }, 0));

        // Mid shift: centre frequency, and depth (negative = gentle notch, 0 = off, positive = boost).
        juce::NormalisableRange<float> midFreqRange (250.0f, 2500.0f, 1.0f);
        midFreqRange.setSkewForCentre (800.0f);
        layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "midfreq", 1 }, "Mid Freq", midFreqRange, 800.0f,
                                                                 juce::AudioParameterFloatAttributes().withLabel ("Hz")));
        layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "middepth", 1 }, "Mid Depth",
                                                                 juce::NormalisableRange<float> (-9.0f, 6.0f, 0.1f), 0.0f,
                                                                 juce::AudioParameterFloatAttributes().withLabel ("dB")));

        return layout;
    }

    void pushParameters()
    {
        engine_.setDrive (driveParam_->load());
        engine_.setTone  (toneParam_->load());
        engine_.setLevel (levelParam_->load());
        engine_.setDiodeType ((int) (diodeParam_->load() + 0.5f));
        engine_.setMidFreq (midFreqParam_->load());
        engine_.setMidGainDb (midDepthParam_->load());
    }

    Klone::KlonCentaurEngine engine_;
    juce::AudioProcessorValueTreeState apvts_;
    std::atomic<float>* driveParam_ = nullptr;
    std::atomic<float>* toneParam_ = nullptr;
    std::atomic<float>* levelParam_ = nullptr;
    std::atomic<float>* enabledParam_ = nullptr;
    std::atomic<float>* diodeParam_ = nullptr;
    std::atomic<float>* midFreqParam_ = nullptr;
    std::atomic<float>* midDepthParam_ = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KlonePrepampProcessor)
};

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new KlonePrepampProcessor();
}
