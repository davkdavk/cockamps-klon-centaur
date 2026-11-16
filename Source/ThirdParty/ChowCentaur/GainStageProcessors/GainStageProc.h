#ifndef GAINSTAGEPROC_H_INCLUDED
#define GAINSTAGEPROC_H_INCLUDED

#include "AmpStage.h"
#include "ClippingStage.h"
#include "FeedForward2.h"
#include "PreAmpStage.h"
#include "SummingAmp.h"

class GainStageProc
{
public:
    GainStageProc (std::atomic<float>* driveParam, double sampleRate);

    void reset (double sampleRate, int samplesPerBlock);
    void processBlock (AudioBuffer<float>& buffer);

    /** 0 = germanium (stock ChowCentaur), 1 = silicon. Call from the audio thread. */
    void setDiodeType (int type)
    {
        const auto t = type == 1 ? GainStageSpace::DiodeType::Silicon : GainStageSpace::DiodeType::Germanium;
        clipL.setDiodeType (t);
        clipR.setDiodeType (t);
    }

private:
    std::atomic<float>* gainParam = nullptr;

    AudioBuffer<float> ff1Buff;
    AudioBuffer<float> ff2Buff;
    dsp::Oversampling<float> os { 2, 1, dsp::Oversampling<float>::FilterType::filterHalfBandPolyphaseIIR };

    GainStageSpace::PreAmpWDF preAmpL, preAmpR;
    GainStageSpace::PreAmpWDF* preAmp[2] { &preAmpL, &preAmpR };

    GainStageSpace::ClippingWDF clipL, clipR;
    GainStageSpace::ClippingWDF* clip[2] { &clipL, &clipR };

    GainStageSpace::FeedForward2WDF ff2L, ff2R;
    GainStageSpace::FeedForward2WDF* ff2[2] { &ff2L, &ff2R };

    GainStageSpace::AmpStage amp[2];
    GainStageSpace::SummingAmp sumAmp[2];
};

#endif // GAINSTAGEPROC_H_INCLUDED
