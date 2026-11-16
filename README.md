# Cockamps Klon Centaur

A JUCE VST3 overdrive plugin with a photo-based pedal editor, three rotary controls, and a footswitch with an active LED.

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
