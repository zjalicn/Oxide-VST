#pragma once

#include <JuceHeader.h>

// Forward declare to avoid circular includes
class OxideAudioProcessor;

class PresetManager
{
public:
    PresetManager(OxideAudioProcessor &processor);
    ~PresetManager();

    // Preset handling methods
    void savePreset(const juce::String &presetName);
    bool loadPreset(const juce::String &presetName);

    // Get preset list
    juce::StringArray getPresetList();

    // Create default presets if needed
    void createDefaultPresetsIfNeeded();

private:
    // Non-owning reference to the processor
    OxideAudioProcessor &processorRef;

    // Directory where presets are stored
    juce::File presetsDirectory;

    // Helper methods
    juce::File getUserPresetsDirectory() const;
    void saveProcessorStateToXml(juce::XmlElement *xml);
    void loadProcessorStateFromXml(const juce::XmlElement *xml);

    // Flag to prevent recursive operations
    bool isInitialized = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetManager)
};