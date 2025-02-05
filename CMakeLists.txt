cmake_minimum_required(VERSION 3.15)
project(HyperDistortion VERSION 1.0.0)

# Find JUCE
add_subdirectory(JUCE)

# Create plugin
juce_add_plugin(HyperDistortion
    VERSION 1.0.0
    COMPANY_NAME "YourCompany"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Manu
    PLUGIN_CODE Dist
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "HyperDistortion")

# Source files
target_sources(HyperDistortion
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp)

# Dependencies
target_compile_features(HyperDistortion PRIVATE cxx_std_17)

target_link_libraries(HyperDistortion
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

# Binary Data
juce_add_binary_data(BinaryData SOURCES
    Resources/background.png
    Resources/knob.png)

target_link_libraries(HyperDistortion PRIVATE BinaryData)
