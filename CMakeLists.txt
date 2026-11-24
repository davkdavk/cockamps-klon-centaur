cmake_minimum_required(VERSION 3.24)

project(KloneCentaur VERSION 1.1.2 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/dependencies/JUCE")

set(KLONE_PLUGIN_FORMATS VST3)
if(APPLE)
    list(APPEND KLONE_PLUGIN_FORMATS AU)
endif()

juce_add_plugin(KloneCentaur
    COMPANY_NAME "Klone"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Klne
    PLUGIN_CODE Klon
    FORMATS ${KLONE_PLUGIN_FORMATS}
    VST3_CATEGORIES Fx Distortion
    PRODUCT_NAME "CockAmps KOCK Centaur")

juce_add_binary_data(KockAssets SOURCES Assets/klon.png)

target_sources(KloneCentaur PRIVATE
    src/mainplugin.cpp
    src/engine/KlonCentaurEngine.h
    Source/KockGUI.cpp
    Source/ThirdParty/ChowCentaur/CommonProcessors/InputBufferProcessor.cpp
    Source/ThirdParty/ChowCentaur/CommonProcessors/InputBufferProcessor.h
    Source/ThirdParty/ChowCentaur/CommonProcessors/OutputStageProcessor.cpp
    Source/ThirdParty/ChowCentaur/CommonProcessors/OutputStageProcessor.h
    Source/ThirdParty/ChowCentaur/CommonProcessors/ToneFilterProcessor.cpp
    Source/ThirdParty/ChowCentaur/CommonProcessors/ToneFilterProcessor.h
    Source/ThirdParty/ChowCentaur/GainStageProcessors/ClippingStage.cpp
    Source/ThirdParty/ChowCentaur/GainStageProcessors/ClippingStage.h
    Source/ThirdParty/ChowCentaur/GainStageProcessors/FeedForward2.cpp
    Source/ThirdParty/ChowCentaur/GainStageProcessors/FeedForward2.h
    Source/ThirdParty/ChowCentaur/GainStageProcessors/GainStageProc.cpp
    Source/ThirdParty/ChowCentaur/GainStageProcessors/GainStageProc.h
    Source/ThirdParty/ChowCentaur/GainStageProcessors/PreAmpStage.cpp
    Source/ThirdParty/ChowCentaur/GainStageProcessors/PreAmpStage.h
    Source/ThirdParty/ChowCentaur/GainStageProcessors/AmpStage.h
    Source/ThirdParty/ChowCentaur/GainStageProcessors/DiodePair.h
    Source/ThirdParty/ChowCentaur/GainStageProcessors/SummingAmp.h
    Source/ThirdParty/ChowCentaur/toms917/toms917.cpp
    Source/ThirdParty/ChowCentaur/toms917/toms917.hpp)

target_include_directories(KloneCentaur PRIVATE
    dependencies/chowdsp_utils
    Source
    Source/ThirdParty/ChowCentaur
    Source/ThirdParty/ChowCentaur/toms917)

target_compile_definitions(KloneCentaur PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(KloneCentaur PRIVATE
    KockAssets
    juce::juce_audio_processors
    juce::juce_dsp
    juce::juce_gui_basics)

target_compile_options(KloneCentaur PRIVATE
    $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:GNU,Clang>>:-O3>)
