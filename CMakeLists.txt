cmake_minimum_required(VERSION 3.15)
project(Oxide VERSION 1.0.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

execute_process(
    COMMAND chmod +x ${CMAKE_CURRENT_SOURCE_DIR}/compile_scss.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(JUCE)

add_custom_target(CompileSCSS
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/compile_scss.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Compiling SCSS to CSS"
    BYPRODUCTS # TODO: Don't think this is working how i thought at first, i kinda fixed it to make sense for now
        ${CMAKE_CURRENT_SOURCE_DIR}/src/resources/layout.css
)

juce_add_binary_data(OxideResources 
    SOURCES
        # Resources
        src/resources/bg.png
        src/resources/layout.html
        src/resources/layout.css
        
        # Fonts
        # src/resources/fonts/old_english_hearts.ttf
)
add_dependencies(OxideResources CompileSCSS)

juce_add_plugin(Oxide
    PRODUCT_NAME "Oxide"
    COMPANY_NAME "createdbyniko."
    FORMATS VST3
    PLUGIN_MANUFACTURER_CODE Niko  # Use your own 4-character code
    PLUGIN_CODE Oxid              # Use your own 4-character code
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_DESCRIPTION "Advanced Multi-Algorithm Distortion"
    VST3_CATEGORIES "Fx,Distortion"
    JUCE_ENABLE_LIVE_CONSTANT_EDITOR=0
    JUCE_USE_WIN_WEBVIEW2=1  # For Windows
    JUCE_USE_WEBKIT=1        # For Mac/iOS
    JUCE_GENERATE_JUCE_HEADER ON
)

juce_generate_juce_header(Oxide)

target_sources(Oxide
    PRIVATE
        # Core
        src/core/PluginProcessor.cpp
        src/core/PluginProcessor.h
        src/core/PluginEditor.cpp
        src/core/PluginEditor.h

        # UI
        src/ui/Background.h
        src/ui/LayoutView.cpp
        src/ui/LayoutView.h

        # DSP
        src/dsp/distortion/DistortionProcessor.cpp
        src/dsp/distortion/DistortionProcessor.h
        src/dsp/delay/DelayProcessor.cpp
        src/dsp/delay/DelayProcessor.h
        src/dsp/filter/FilterProcessor.cpp
        src/dsp/filter/FilterProcessor.h
        src/dsp/pulse/PulseProcessor.cpp
        src/dsp/pulse/PulseProcessor.h
)

target_include_directories(Oxide
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
        ${CMAKE_CURRENT_SOURCE_DIR}/src/dsp/distortion
        ${CMAKE_CURRENT_SOURCE_DIR}/src/dsp/delay
        ${CMAKE_CURRENT_SOURCE_DIR}/src/dsp/filter
        ${CMAKE_CURRENT_SOURCE_DIR}/src/dsp/pulse
)

target_compile_definitions(Oxide
    PRIVATE
        JUCE_WEB_BROWSER=1
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:Oxide,PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:Oxide,VERSION>"
)

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