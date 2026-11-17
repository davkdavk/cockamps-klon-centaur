# Cockamps Klon Centaur

A JUCE VST3 overdrive plugin using the Traditional wave-digital-filter circuit model from ChowCentaur, with a custom photo editor, three rotary controls, and a working footswitch/LED.

## Build

Initialize the JUCE submodule, then configure and build with CMake:

```sh
git submodule update --init --recursive
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target KloneCentaur_VST3 -j
```

The Linux VST3 bundle is generated at:

```text
build/KloneCentaur_artefacts/Release/VST3/CockAmps KOCK Centaur.vst3
```

## Controls

- **Gain** controls drive.
- **Treble** controls tone.
- **Output** controls output level.
- **Footswitch** toggles effect bypass; the red LED is lit while the effect is enabled.

- **Diode** (host parameter) switches the clipping diodes between **Germanium** (the stock ChowCentaur model, default) and **Silicon** (1N4148-style: later, harder-edged breakup and a more open, louder response; a fixed -6 dB trim keeps the loudness in the same ballpark). The values live in `Source/ThirdParty/ChowCentaur/GainStageProcessors/ClippingStage.h` (`getDiodeModel`) and `siliconTrimDb` in `src/engine/KlonCentaurEngine.h`.
- **Mid Freq** (250 Hz-2.5 kHz) and **Mid Depth** (-9 dB to +6 dB, default 0 dB = off) (host parameters) add a wide, gentle mid notch/boost after the pedal model, with the centre frequency adjustable.

The three continuous controls retain their first three parameter slots. The enable/bypass parameter is appended after them, followed by Diode, Mid Freq and Mid Depth.

## DSP and Licensing

The signal path adapts ChowCentaur's circuit-derived input, gain/clipping, treble, and output stages. The gain/clipping stage uses the upstream 2x oversampled WDF model; Drive remains mapped to its gain control, and Treble/Output map to the corresponding modeled stages. This is a circuit-based model, not a claim of measured unit-by-unit accuracy.

This distribution is AGPL-3.0-only; see `LICENSE`. Third-party source provenance, modifications, and license notices are documented in `ThirdPartyLicenses/README.md`.
