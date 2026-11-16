/*
    Host adapter around ChowCentaur's Traditional circuit-modeling stages.
    The DSP sources are vendored under Source/ThirdParty/ChowCentaur and
    credited/licensed in ThirdPartyLicenses/.
*/

#pragma once

#include <juce_dsp/juce_dsp.h>
#include <atomic>
#include <memory>

#include "CommonProcessors/InputBufferProcessor.h"
#include "CommonProcessors/OutputStageProcessor.h"
#include "CommonProcessors/ToneFilterProcessor.h"
#include "GainStageProcessors/GainStageProc.h"

namespace Klone
{

class KlonCentaurEngine
{
public:
    void prepare (double sampleRate, int samplesPerBlock)
    {
        sampleRate_ = sampleRate;
        gainStage_ = std::make_unique<GainStageProc> (&drive_, sampleRate_);
        gainStage_->reset (sampleRate_, samplesPerBlock);
        gainStage_->setDiodeType (diodeType_);

        for (int ch = 0; ch < 2; ++ch)
        {
            input_[ch].prepare ((float) sampleRate_);
            tone_[ch].prepare ((float) sampleRate_);
            output_[ch].prepare ((float) sampleRate_);
        }

        *dcBlocker_.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass (sampleRate_, 35.0f);
        dcBlocker_.prepare ({ sampleRate_, (juce::uint32) samplesPerBlock, 2 });

        midFreqSmooth_.reset (sampleRate_, 0.05);
        midFreqSmooth_.setCurrentAndTargetValue (midFreq_.load());
        midGainSmooth_.reset (sampleRate_, 0.05);
        midGainSmooth_.setCurrentAndTargetValue (midGainDb_.load());
        midFilter_.prepare ({ sampleRate_, (juce::uint32) samplesPerBlock, 2 });
        updateMidCoefficients();
    }

    /** Mid shift: centre frequency in Hz (roughly 250 Hz - 2.5 kHz). */
    void setMidFreq (float hz) { midFreq_.store (juce::jlimit (100.0f, 6000.0f, hz), std::memory_order_relaxed); }

    /** Mid depth in dB: negative = gentle notch, positive = boost, 0 = off (bit-exact bypass). */
    void setMidGainDb (float db) { midGainDb_.store (juce::jlimit (-18.0f, 12.0f, db), std::memory_order_relaxed); }

    void setDrive (float value) { drive_.store (clamp01 (value), std::memory_order_relaxed); }
    void setTone (float value)  { toneValue_.store (clamp01 (value), std::memory_order_relaxed); }
    void setLevel (float value) { level_.store (clamp01 (value), std::memory_order_relaxed); }

    /** 0 = germanium (stock), 1 = silicon. Audio thread only. */
    void setDiodeType (int type)
    {
        type = type == 1 ? 1 : 0;
        if (type == diodeType_)
            return;

        diodeType_ = type;
        if (gainStage_ != nullptr)
            gainStage_->setDiodeType (diodeType_);
    }

    void processBlock (juce::AudioBuffer<float>& buffer)
    {
        if (gainStage_ == nullptr || buffer.getNumChannels() != 2)
            return;

        const int numSamples = buffer.getNumSamples();
        for (int ch = 0; ch < 2; ++ch)
        {
            auto* samples = buffer.getWritePointer (ch);
            juce::FloatVectorOperations::multiply (samples, 0.5f, numSamples);
            input_[ch].processBlock (samples, numSamples);
            juce::FloatVectorOperations::clip (samples, samples, -4.5f, 4.5f, numSamples);
        }

        gainStage_->processBlock (buffer);

        for (int ch = 0; ch < 2; ++ch)
        {
            auto* samples = buffer.getWritePointer (ch);
            tone_[ch].setTreble (toneValue_.load (std::memory_order_relaxed));
            tone_[ch].processBlock (samples, numSamples);
            juce::FloatVectorOperations::multiply (samples, -1.0f, numSamples);
            juce::FloatVectorOperations::clip (samples, samples, -13.1f, 11.7f, numSamples);
            output_[ch].setLevel (level_.load (std::memory_order_relaxed));
            output_[ch].processBlock (samples, numSamples);
        }

        juce::dsp::AudioBlock<float> block (buffer);
        juce::dsp::ProcessContextReplacing<float> context (block);
        dcBlocker_.process (context);

        // Silicon clips much later than germanium and comes out several dB louder, so it gets a fixed
        // trim (siliconTrimDb) to keep the two settings in the same loudness ballpark. Ramped over the
        // block when the diode is switched to avoid a step.
        {
            const float trimTarget = diodeType_ == 1 ? juce::Decibels::decibelsToGain (siliconTrimDb) : 1.0f;
            if (trimTarget != diodeTrim_)
            {
                buffer.applyGainRamp (0, numSamples, diodeTrim_, trimTarget);
                diodeTrim_ = trimTarget;
            }
            else if (diodeTrim_ != 1.0f)
            {
                buffer.applyGain (diodeTrim_);
            }
        }

        // Mid notch / shift: a wide, gentle peaking filter after the pedal model.
        // Skipped entirely at 0 dB, so the stock sound is untouched.
        midFreqSmooth_.setTargetValue (midFreq_.load (std::memory_order_relaxed));
        midGainSmooth_.setTargetValue (midGainDb_.load (std::memory_order_relaxed));
        const bool moving = midFreqSmooth_.isSmoothing() || midGainSmooth_.isSmoothing();
        if (moving)
        {
            midFreqSmooth_.skip (numSamples);
            midGainSmooth_.skip (numSamples);
            updateMidCoefficients();
        }

        if (moving || std::abs (midGainSmooth_.getTargetValue()) > 0.01f)
            midFilter_.process (context);
    }

private:
    static float clamp01 (float value) { return juce::jlimit (0.0f, 1.0f, value); }

    void updateMidCoefficients()
    {
        constexpr float q = 0.7f; // low Q = wide, gentle
        const float freq = juce::jmin (midFreqSmooth_.getCurrentValue(), (float) sampleRate_ * 0.45f);
        *midFilter_.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter (
            sampleRate_, freq, q, juce::Decibels::decibelsToGain (midGainSmooth_.getCurrentValue()));
    }

    double sampleRate_ = 44100.0;
    std::atomic<float> drive_ { 0.5f };
    std::atomic<float> toneValue_ { 0.5f };
    std::atomic<float> level_ { 0.5f };
    int diodeType_ = 0;
    static constexpr float siliconTrimDb = -6.0f; // tuning constant: output trim applied in Silicon mode
    float diodeTrim_ = 1.0f;
    std::atomic<float> midFreq_ { 800.0f };
    std::atomic<float> midGainDb_ { 0.0f };
    juce::SmoothedValue<float> midFreqSmooth_ { 800.0f };
    juce::SmoothedValue<float> midGainSmooth_ { 0.0f };

    std::unique_ptr<GainStageProc> gainStage_;
    InputBufferProcessor input_[2];
    ToneFilterProcessor tone_[2];
    OutputStageProc output_[2];

    using StereoIIR = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
                                                      juce::dsp::IIR::Coefficients<float>>;
    StereoIIR dcBlocker_;
    StereoIIR midFilter_;
};

} // namespace Klone
