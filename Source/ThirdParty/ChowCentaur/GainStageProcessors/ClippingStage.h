#ifndef CLIPPINGSTAGE_H_INCLUDED
#define CLIPPINGSTAGE_H_INCLUDED

#include "DiodePair.h"

namespace GainStageSpace
{
using namespace chowdsp::WDFT;

/** Diode models for the clipping pair (modification for Klone Centaur Preamp).
 *  Is = reverse saturation current, Vt = n * kT/q (emission coefficient folded in).
 *  Germanium is the stock ChowCentaur value, so the default sound is unchanged.
 *  Silicon is a typical 1N4148 SPICE model (Is = 2.52 nA, n = 1.752).
 *  These are tuning constants: edit freely.
 */
enum class DiodeType
{
    Germanium = 0,
    Silicon = 1,
};

struct DiodeModel
{
    double Is;
    double Vt;
};

constexpr double thermalVoltage = 0.02585;

constexpr DiodeModel getDiodeModel (DiodeType type)
{
    return type == DiodeType::Silicon ? DiodeModel { 2.52e-9, 1.752 * thermalVoltage }
                                      : DiodeModel { 15.0e-6, thermalVoltage };
}

class ClippingWDF
{
public:
    ClippingWDF (double sampleRate);

    void reset();

    void setDiodeType (DiodeType type)
    {
        const auto m = getDiodeModel (type);
        D23.setDiode (m.Is, m.Vt);
    }

    inline float processSample (float x)
    {
        Vin.setVoltage ((double) x);

        D23.incident (P1.reflected());
        P1.incident (D23.reflected());
        auto y = current<double> (C10);

        return (float) y;
    }

private:
    using Capacitor = CapacitorT<double>;
    using Resistor = ResistorT<double>;
    using ResVs = ResistiveVoltageSourceT<double>;

    ResVs Vin;
    Capacitor C9;
    Resistor R13 { 1000.0 };

    Capacitor C10;
    ResVs Vbias { 47000.0 };

    PolarityInverterT<double, ResVs> I1 { Vin };
    WDFSeriesT<double, decltype (I1), Capacitor> S1 { I1, C9 };
    WDFSeriesT<double, decltype (S1), Resistor> S2 { S1, R13 };

    WDFSeriesT<double, Capacitor, ResVs> S3 { C10, Vbias };
    WDFParallelT<double, decltype (S2), decltype (S3)> P1 { S2, S3 };

    CustomDiodePairT<double, decltype (P1)> D23 { getDiodeModel (DiodeType::Germanium).Is,
                                                  getDiodeModel (DiodeType::Germanium).Vt,
                                                  P1 };
};

} // namespace GainStageSpace

#endif // CLIPPINGSTAGE_H_INCLUDED
