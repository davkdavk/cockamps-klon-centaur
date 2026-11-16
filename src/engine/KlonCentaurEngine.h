/* ==============================================================================
   Klon-style overdrive DSP engine (pure C++, no JUCE dependency)

   Signal chain (per channel):
     pre-gain (Drive) -> asymmetric soft clip -> mid hump (~720 Hz peaking)
     -> tone (one-pole low-pass, dark <-> bright) -> DC blocker -> level
   ============================================================================== */

#pragma once

#include <algorithm>
#include <cmath>

namespace Klone
{

constexpr double kPi = 3.14159265358979323846;

class KlonCentaurEngine
{
public:
    // Parameter ranges are normalised 0..1
    void setSampleRate (double sr)
    {
        sampleRate_ = (sr > 1000.0) ? sr : 44100.0;
        updateCoefficients();
        reset();
    }

    void setDrive (float d) { drive_ = clamp01 (d); }

    void setTone (float t)
    {
        tone_ = clamp01 (t);
        updateCoefficients();
    }

    void setLevel (float l) { level_ = 2.0f * clamp01 (l); }  // 0..2 linear

    void reset()
    {
        for (auto& s : state_)
        {
            s.mid.resetState();
            s.lpState = 0.0f;
            s.dcX = 0.0f;
            s.dcY = 0.0f;
        }
    }

    // Process one stereo frame in place
    void processStereoFrame (float& left, float& right)
    {
        left  = processSample (left,  state_[0], false);
        right = processSample (right, state_[1], true);
    }

private:
    static float clamp01 (float x) { return std::max (0.0f, std::min (1.0f, x)); }

    struct Biquad
    {
        float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f, a1 = 0.0f, a2 = 0.0f;
        float x1 = 0.0f, x2 = 0.0f, y1 = 0.0f, y2 = 0.0f;

        float process (float x)
        {
            const float y = b0 * x + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
            x2 = x1; x1 = x;
            y2 = y1; y1 = y;
            return y;
        }

        void resetState() { x1 = x2 = y1 = y2 = 0.0f; }
    };

    struct ChannelState
    {
        Biquad mid;
        float lpState = 0.0f;
        float dcX = 0.0f, dcY = 0.0f;
    };

    void updateCoefficients()
    {
        // RBJ peaking EQ: ~5 dB hump at 720 Hz, Q 0.9
        const double fc = 720.0;
        const double q = 0.9;
        const double dB = 5.0;
        const double A = std::pow (10.0, dB / 40.0);
        const double w0 = 2.0 * kPi * fc / sampleRate_;
        const double alpha = std::sin (w0) / (2.0 * q);
        const double cw = std::cos (w0);

        const double a0 = 1.0 + alpha / A;
        const float b0 = (float) ((1.0 + alpha * A) / a0);
        const float b1 = (float) (-2.0 * cw / a0);
        const float b2 = (float) ((1.0 - alpha * A) / a0);
        const float a1 = (float) (-2.0 * cw / a0);
        const float a2 = (float) ((1.0 - alpha / A) / a0);

        for (auto& s : state_)
        {
            s.mid.b0 = b0; s.mid.b1 = b1; s.mid.b2 = b2;
            s.mid.a1 = a1; s.mid.a2 = a2;
        }

        // Tone: low-pass cutoff sweeps 1.5 kHz (tone 0) -> 12 kHz (tone 1)
        const double cutoff = 1500.0 * std::pow (8.0, (double) tone_);
        lpCoeff_ = (float) std::exp (-2.0 * kPi * cutoff / sampleRate_);
    }

    float processSample (float in, ChannelState& s, bool mirrored)
    {
        // Drive: 0 dB .. +40 dB pre-gain
        const float pre = std::pow (10.0f, 2.0f * drive_) * 0.25f;
        const float x = in * pre;

        // Asymmetric soft clipping; the right channel mirrors the slopes
        const float kPos = mirrored ? 0.8f : 1.0f;
        const float kNeg = mirrored ? 1.0f : 0.8f;
        float y = (x >= 0.0f) ? std::tanh (x * kPos) / kPos
                              : std::tanh (x * kNeg) / kNeg;

        y = s.mid.process (y);

        s.lpState = (1.0f - lpCoeff_) * y + lpCoeff_ * s.lpState;
        y = s.lpState;

        // DC blocker (asymmetric clipping introduces DC offset)
        const float dc = y - s.dcX + 0.995f * s.dcY;
        s.dcX = y;
        s.dcY = dc;
        y = dc * level_;

        return std::max (-1.0f, std::min (1.0f, y));
    }

    double sampleRate_ = 44100.0;
    float drive_ = 0.5f;
    float tone_ = 0.5f;
    float level_ = 1.0f;
    float lpCoeff_ = 0.0f;
    ChannelState state_[2];
};

} // namespace Klone
