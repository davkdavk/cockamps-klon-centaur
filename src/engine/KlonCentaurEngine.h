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

        for (int ch = 0; ch < 2; ++ch)
        {
            input_[ch].prepare ((float) sampleRate_);
            tone_[ch].prepare ((float) sampleRate_);
            output_[ch].prepare ((float) sampleRate_);
        }

        *dcBlocker_.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass (sampleRate_, 35.0f);
        dcBlocker_.prepare ({ sampleRate_, (juce::uint32) samplesPerBlock, 2 });
    }

    void setDrive (float value) { drive_.store (clamp01 (value), std::memory_order_relaxed); }
    void setTone (float value)  { toneValue_.store (clamp01 (value), std::memory_order_relaxed); }
    void setLevel (float value) { level_.store (clamp01 (value), std::memory_order_relaxed); }

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
    }

private:
    static float clamp01 (float value) { return juce::jlimit (0.0f, 1.0f, value); }

    double sampleRate_ = 44100.0;
    std::atomic<float> drive_ { 0.5f };
    std::atomic<float> toneValue_ { 0.5f };
    std::atomic<float> level_ { 0.5f };

    std::unique_ptr<GainStageProc> gainStage_;
    InputBufferProcessor input_[2];
    ToneFilterProcessor tone_[2];
    OutputStageProc output_[2];

    using StereoIIR = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
                                                      juce::dsp::IIR::Coefficients<float>>;
    StereoIIR dcBlocker_;
};

} // namespace Klone
