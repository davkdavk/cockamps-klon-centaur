cmake_minimum_required(VERSION 3.24)

project(KloneCentaur VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/dependencies/JUCE")

juce_add_plugin(KloneCentaur
    COMPANY_NAME "Klone"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Klne
    PLUGIN_CODE Klon
    FORMATS VST3
    VST3_CATEGORIES Fx Distortion
    PRODUCT_NAME "Klone Centaur Preamp")

juce_add_binary_data(KockAssets SOURCES Assets/klon.png)

target_sources(KloneCentaur PRIVATE
    src/mainplugin.cpp
    src/engine/KlonCentaurEngine.h
    Source/KockGUI.cpp)

target_include_directories(KloneCentaur PRIVATE Source)

target_compile_definitions(KloneCentaur PUBLIC
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(KloneCentaur PRIVATE
    KockAssets
    juce::juce_audio_processors
    juce::juce_gui_basics)

target_compile_options(KloneCentaur PRIVATE
    $<$<CONFIG:Release>:-O3>)
