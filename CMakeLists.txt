cmake_minimum_required(VERSION 3.15)
project(Oxide VERSION 1.0.0)

# Enable compile_commands.json generation
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use JUCE from the git submodule
add_subdirectory(JUCE)

juce_add_binary_data(OxideResources 
    SOURCES
        Resources/meter.html
        Resources/controlPanel.html
)

# Create the plugin target
juce_add_plugin(Oxide
    PRODUCT_NAME "Oxide"
    COMPANY_NAME "createdbyniko."
    FORMATS VST3               # Specify VST3 format
    PLUGIN_MANUFACTURER_CODE Niko  # Use your own 4-character code
    PLUGIN_CODE Oxid              # Use your own 4-character code
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD TRUE    # Automatically copy to your VST folder
    PLUGIN_DESCRIPTION "Advanced Multi-Algorithm Distortion"
    VST3_CATEGORIES "Fx,Distortion" # VST3 categories
    JUCE_ENABLE_LIVE_CONSTANT_EDITOR=0
    JUCE_USE_WIN_WEBVIEW2=1  # For Windows
    JUCE_USE_WEBKIT=1        # For Mac/iOS
    JUCE_GENERATE_JUCE_HEADER ON
)

juce_generate_juce_header(Oxide)

# Add source files
target_sources(Oxide
    PRIVATE
        # Core
        Source/Core/PluginProcessor.cpp
        Source/Core/PluginProcessor.h
        Source/Core/PluginEditor.cpp
        Source/Core/PluginEditor.h

        # UI
        Source/UI/ControlPanel.cpp
        Source/UI/ControlPanel.h
        Source/UI/MeterView.cpp
        Source/UI/MeterView.h

        # FX
        Source/FX/Distortion/DistortionProcessor.cpp
        Source/FX/Distortion/DistortionProcessor.h
)

# Include directories
target_include_directories(Oxide
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/Core
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/UI
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/FX/Distortion
        ${CMAKE_CURRENT_BINARY_DIR}/Oxide_artefacts/JuceLibraryCode
)

# Add JUCE modules
target_compile_definitions(Oxide
    PRIVATE
        JUCE_WEB_BROWSER=1
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0  # Add this if you don't have VST2 license
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:Oxide,PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:Oxide,VERSION>"
)

# Link to JUCE modules
target_link_libraries(Oxide
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        OxideResources
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)