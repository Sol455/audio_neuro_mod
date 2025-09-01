cmake_minimum_required(VERSION 3.24)

# Change to your project name
project(audio_neuro_mod VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_XCODE_GENERATE_SCHEME OFF)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(external/JUCE)

add_subdirectory(external/liblsl)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Source)

# Make sure you include any new source files here
set(SourceFiles
        ${SRC_DIR}/PluginEditor.cpp
        ${SRC_DIR}/PluginEditor.h
        ${SRC_DIR}/PluginProcessor.cpp
        ${SRC_DIR}/PluginProcessor.h
        ${SRC_DIR}/Carrier.cpp
        ${SRC_DIR}/Carrier.h
        ${SRC_DIR}/Params.h
        ${SRC_DIR}/lsl/lsl_connector.h
        ${SRC_DIR}/lsl/eegRingBuffer.h
        ${SRC_DIR}/lsl/lsl_worker.h
        ${SRC_DIR}/dsp_worker.cpp
        ${SRC_DIR}/dsp_worker.h
        ${SRC_DIR}/filterMod.cpp
        ${SRC_DIR}/filterMod.h
        ${SRC_DIR}/EegScopeComponent.h
        ${SRC_DIR}/MidiOutputLayer.h
)

# Change these to your own preferences
juce_add_plugin(${PROJECT_NAME}
        COMPANY_NAME sol
        BUNDLE_ID com.solharter.audio_neuro_mod
        IS_SYNTH TRUE
        NEEDS_MIDI_INPUT TRUE
        NEEDS_MIDI_OUTPUT TRUE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        JUCE_VST3_CAN_REPLACE_VST2 FALSE
        COPY_PLUGIN_AFTER_BUILD TRUE
        PLUGIN_MANUFACTURER_CODE Tap1
        PLUGIN_CODE Reg0
        FORMATS VST3 AU Standalone
        PRODUCT_NAME "AudioNeuroMod"
        VST3_CATEGORIES "Instrument|Synth"
)

# How we want our SourceFiles to appear in our IDE
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SourceFiles})

# Make the SourceFiles buildable
target_sources(${PROJECT_NAME} PRIVATE ${SourceFiles})

# These are some toggleable options from the JUCE CMake API
target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

# JUCE libraries to bring into our project
target_link_libraries(${PROJECT_NAME}
        PUBLIC
        juce::juce_analytics
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_core
        juce::juce_data_structures
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
        lsl
)

