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
build/KloneCentaur_artefacts/Release/VST3/Klone Centaur Preamp.vst3
```

## Controls

- **Gain** controls drive.
- **Treble** controls tone.
- **Output** controls output level.
- **Footswitch** toggles effect bypass; the red LED is lit while the effect is enabled.

The three continuous controls retain their first three parameter slots. The enable/bypass parameter is appended after them.

## DSP and Licensing

The signal path adapts ChowCentaur's circuit-derived input, gain/clipping, treble, and output stages. The gain/clipping stage uses the upstream 2x oversampled WDF model; Drive remains mapped to its gain control, and Treble/Output map to the corresponding modeled stages. This is a circuit-based model, not a claim of measured unit-by-unit accuracy.

This distribution is AGPL-3.0-only; see `LICENSE`. Third-party source provenance, modifications, and license notices are documented in `ThirdPartyLicenses/README.md`.
