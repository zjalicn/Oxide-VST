cmake_minimum_required(VERSION 3.15)
project(Oxide VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use JUCE from the git submodule
add_subdirectory(JUCE)

# Create the main target
juce_add_gui_app(Oxide
    PRODUCT_NAME "Oxide"
    COMPANY_NAME "YourCompany"
    JUCE_GENERATE_JUCE_HEADER ON  # This will generate JuceHeader.h
)

# Add source files
target_sources(Oxide
    PRIVATE
        Source/Main.cpp
        Source/MainComponent.cpp
        Source/MainComponent.h
        Source/MeterView.cpp
        Source/MeterView.h
        Source/ControlPanel.cpp
        Source/ControlPanel.h
        Source/DistortionProcessor.cpp
        Source/DistortionProcessor.h
)

# Include directories
target_include_directories(Oxide
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
        ${CMAKE_CURRENT_BINARY_DIR}/JuceLibraryCode
)

# Add JUCE modules
target_compile_definitions(Oxide
    PRIVATE
        JUCE_WEB_BROWSER=1
        JUCE_USE_CURL=0
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
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)