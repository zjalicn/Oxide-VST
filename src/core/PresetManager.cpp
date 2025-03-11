#include "PresetManager.h"
#include "PluginProcessor.h"

PresetManager::PresetManager(OxideAudioProcessor &processor)
    : processorRef(processor), isInitialized(false)
{
    // Get the presets directory
    presetsDirectory = getUserPresetsDirectory();

    // Create the directory if it doesn't exist
    if (!presetsDirectory.exists())
    {
        presetsDirectory.createDirectory();
    }

    isInitialized = true;

    // Create default presets if none exist
    if (presetsDirectory.getNumberOfChildFiles(juce::File::findFiles) == 0)
    {
        createDefaultPresetsIfNeeded();
    }
}

PresetManager::~PresetManager()
{
    // Nothing to clean up
}

void PresetManager::savePreset(const juce::String &presetName)
{
    if (!isInitialized)
    {
        return;
    }

    // Create a new XML element
    auto xml = std::make_unique<juce::XmlElement>("OxidePreset");
    xml->setAttribute("name", presetName);
    xml->setAttribute("version", "1.0");

    // Save processor state
    saveProcessorStateToXml(xml.get());

    // Save to file
    juce::File presetFile = presetsDirectory.getChildFile(presetName + ".xml");
    xml->writeToFile(presetFile, "");
}

bool PresetManager::loadPreset(const juce::String &presetName)
{
    if (!isInitialized)
    {
        return false;
    }

    // Find the preset file
    juce::File presetFile = presetsDirectory.getChildFile(presetName + ".xml");

    if (!presetFile.existsAsFile())
    {
        // Try with .xml extension if user included it
        if (presetName.endsWith(".xml"))
        {
            presetFile = presetsDirectory.getChildFile(presetName);
        }

        // Try with underscore instead of spaces
        if (!presetFile.existsAsFile() && presetName.contains("_"))
        {
            juce::String spacedName = presetName.replaceCharacters("_", " ");
            presetFile = presetsDirectory.getChildFile(spacedName + ".xml");

            // If still not found, try case-insensitive search
            if (!presetFile.existsAsFile())
            {
                // Get all XML files
                juce::Array<juce::File> allXmlFiles = presetsDirectory.findChildFiles(
                    juce::File::findFiles, false, "*.xml");

                // Look for a case-insensitive match
                for (const auto &file : allXmlFiles)
                {
                    if (file.getFileNameWithoutExtension().toLowerCase() == spacedName.toLowerCase())
                    {
                        presetFile = file;
                        break;
                    }
                }
            }
        }

        if (!presetFile.existsAsFile())
        {
            // As a last resort, try direct case-insensitive search
            juce::Array<juce::File> allXmlFiles = presetsDirectory.findChildFiles(
                juce::File::findFiles, false, "*.xml");

            for (const auto &file : allXmlFiles)
            {
                if (file.getFileNameWithoutExtension().toLowerCase() == presetName.toLowerCase())
                {
                    presetFile = file;
                    break;
                }
            }
        }

        if (!presetFile.existsAsFile())
        {
            return false;
        }
    }

    // Parse the XML file
    std::unique_ptr<juce::XmlElement> xml = juce::XmlDocument::parse(presetFile);
    if (xml == nullptr || xml->getTagName() != "OxidePreset")
    {
        return false;
    }

    // Load processor state
    loadProcessorStateFromXml(xml.get());

    return true;
}

juce::StringArray PresetManager::getPresetList()
{
    juce::StringArray presetList;

    // Initial default preset if not initialized yet
    if (!isInitialized)
    {
        presetList.add("Default");
        return presetList;
    }

    // Find all XML files in the presets directory
    juce::Array<juce::File> presetFiles = presetsDirectory.findChildFiles(
        juce::File::findFiles, false, "*.xml");

    // Add each preset name to the list
    for (const auto &file : presetFiles)
    {
        presetList.add(file.getFileNameWithoutExtension());
    }

    // Add a default preset if none found
    if (presetList.isEmpty())
    {
        presetList.add("Default");
    }

    return presetList;
}

juce::File PresetManager::getUserPresetsDirectory() const
{
    juce::File dir;

#if JUCE_MAC
    // Mac: ~/Library/Application Support/Oxide/Presets
    dir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
              .getChildFile("Application Support")
              .getChildFile("Oxide")
              .getChildFile("Presets");
#elif JUCE_WINDOWS
    // Windows: %APPDATA%\Oxide\Presets
    dir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
              .getChildFile("Oxide")
              .getChildFile("Presets");
#else
    // Linux: ~/.config/Oxide/Presets
    dir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
              .getChildFile(".config")
              .getChildFile("Oxide")
              .getChildFile("Presets");
#endif

    return dir;
}

void PresetManager::saveProcessorStateToXml(juce::XmlElement *xml)
{
    auto &distortion = processorRef.getDistortionProcessor();
    auto &delay = processorRef.getDelayProcessor();
    auto &filter = processorRef.getFilterProcessor();
    auto &pulse = processorRef.getPulseProcessor();

    // Create sections for each processor
    auto distortionXml = xml->createNewChildElement("Distortion");
    auto delayXml = xml->createNewChildElement("Delay");
    auto filterXml = xml->createNewChildElement("Filter");
    auto pulseXml = xml->createNewChildElement("Pulse");

    // Distortion parameters
    distortionXml->setAttribute("drive", distortion.getDrive());
    distortionXml->setAttribute("mix", distortion.getMix());
    distortionXml->setAttribute("inputGain", distortion.getInputGain());
    distortionXml->setAttribute("outputGain", distortion.getOutputGain());
    distortionXml->setAttribute("algorithm", distortion.getAlgorithmName());

    // Delay parameters
    delayXml->setAttribute("time", delay.getDelayTime());
    delayXml->setAttribute("feedback", delay.getFeedback());
    delayXml->setAttribute("mix", delay.getMix());
    delayXml->setAttribute("pingPong", delay.getPingPong());

    // Filter parameters
    filterXml->setAttribute("type", filter.getFilterTypeName());
    filterXml->setAttribute("frequency", filter.getFrequency());
    filterXml->setAttribute("resonance", filter.getResonance());

    // Pulse parameters
    pulseXml->setAttribute("mix", pulse.getMix());
    pulseXml->setAttribute("rate", pulse.getRateString());
}

void PresetManager::loadProcessorStateFromXml(const juce::XmlElement *xml)
{
    auto &distortion = processorRef.getDistortionProcessor();
    auto &delay = processorRef.getDelayProcessor();
    auto &filter = processorRef.getFilterProcessor();
    auto &pulse = processorRef.getPulseProcessor();

    // Extract distortion parameters
    if (auto *distortionXml = xml->getChildByName("Distortion"))
    {
        if (distortionXml->hasAttribute("drive"))
            distortion.setDrive(distortionXml->getDoubleAttribute("drive"));

        if (distortionXml->hasAttribute("mix"))
            distortion.setMix(distortionXml->getDoubleAttribute("mix"));

        if (distortionXml->hasAttribute("inputGain"))
            distortion.setInputGain(distortionXml->getDoubleAttribute("inputGain"));

        if (distortionXml->hasAttribute("outputGain"))
            distortion.setOutputGain(distortionXml->getDoubleAttribute("outputGain"));

        if (distortionXml->hasAttribute("algorithm"))
        {
            // Explicitly call the string version
            juce::String algoStr = distortionXml->getStringAttribute("algorithm");
            distortion.setAlgorithm(algoStr);
        }
    }

    // Extract delay parameters
    if (auto *delayXml = xml->getChildByName("Delay"))
    {
        if (delayXml->hasAttribute("time"))
            delay.setDelayTime(delayXml->getDoubleAttribute("time"));

        if (delayXml->hasAttribute("feedback"))
            delay.setFeedback(delayXml->getDoubleAttribute("feedback"));

        if (delayXml->hasAttribute("mix"))
            delay.setMix(delayXml->getDoubleAttribute("mix"));

        if (delayXml->hasAttribute("pingPong"))
            delay.setPingPong(delayXml->getBoolAttribute("pingPong"));
    }

    // Extract filter parameters
    if (auto *filterXml = xml->getChildByName("Filter"))
    {
        if (filterXml->hasAttribute("type"))
        {
            // Explicitly call the string version
            juce::String typeStr = filterXml->getStringAttribute("type");
            filter.setFilterType(typeStr);
        }

        if (filterXml->hasAttribute("frequency"))
            filter.setFrequency(filterXml->getDoubleAttribute("frequency"));

        if (filterXml->hasAttribute("resonance"))
            filter.setResonance(filterXml->getDoubleAttribute("resonance"));
    }

    // Extract pulse parameters
    if (auto *pulseXml = xml->getChildByName("Pulse"))
    {
        if (pulseXml->hasAttribute("mix"))
            pulse.setMix(pulseXml->getDoubleAttribute("mix"));

        if (pulseXml->hasAttribute("rate"))
            pulse.setRate(pulseXml->getStringAttribute("rate"));
    }
}

void PresetManager::createDefaultPresetsIfNeeded()
{
    // Don't create presets if we're not initialized
    if (!isInitialized)
    {
        return;
    }

    // If there are already presets, don't create defaults
    if (presetsDirectory.getNumberOfChildFiles(juce::File::findFiles) > 0)
    {
        return;
    }

    // Create a temporary XML to store the current state
    auto currentState = std::make_unique<juce::XmlElement>("CurrentState");
    saveProcessorStateToXml(currentState.get());

    // Define the default presets
    struct PresetData
    {
        juce::String name;
        float drive, mix, inputGain, outputGain;
        juce::String algorithm;
        float delayTime, feedback, delayMix;
        bool pingPong;
        juce::String filterType;
        float frequency, resonance;
        float pulseMix;
        juce::String pulseRate;
    };

    PresetData presets[] = {
        {"Default", 0.5f, 0.5f, 0.0f, 0.0f, "soft_clip", 0.5f, 0.4f, 0.3f, false, "lowpass", 1000.0f, 0.7f, 0.0f, "1/4"},
        {"Light Drive", 0.3f, 0.5f, 0.2f, 0.5f, "soft_clip", 0.5f, 0.4f, 0.3f, false, "lowpass", 1200.0f, 0.5f, 0.2f, "1/4"},
        {"Heavy Metal", 0.8f, 0.7f, 3.0f, 2.0f, "hard_clip", 0.5f, 0.5f, 0.5f, true, "lowpass", 2000.0f, 1.2f, 0.4f, "1/4"},
        {"Fuzz", 1.0f, 1.0f, 6.0f, -3.0f, "foldback", 0.7f, 0.7f, 0.6f, false, "highpass", 500.0f, 0.8f, 1.0f, "1/4"},
        {"Warm Tape", 0.5f, 0.4f, 1.5f, 0.0f, "waveshaper", 0.3f, 0.6f, 0.4f, true, "bandpass", 1500.0f, 2.0f, 0.6f, "1/4"},
        {"Subtle Texture", 0.25f, 0.35f, 1.0f, 0.0f, "waveshaper", 0.2f, 0.25f, 0.2f, true, "highpass", 400.0f, 0.5f, 0.15f, "1/8"},
        {"Rhythmic Grind", 0.7f, 0.8f, 2.5f, -1.5f, "hard_clip", 0.25f, 0.6f, 0.5f, true, "bandpass", 1200.0f, 3.0f, 0.8f, "1/8"},
        {"Analog Crush", 0.9f, 0.65f, 4.0f, -2.0f, "bitcrusher", 0.15f, 0.3f, 0.25f, false, "lowpass", 1800.0f, 1.0f, 0.3f, "1/4"},
        {"Ambient Wash", 0.4f, 0.6f, 1.0f, 0.5f, "soft_clip", 0.9f, 0.75f, 0.8f, true, "lowpass", 3000.0f, 0.3f, 0.2f, "1/2"},
        {"Bass Thickener", 0.35f, 0.55f, 3.0f, -1.0f, "foldback", 0.1f, 0.2f, 0.15f, false, "lowpass", 500.0f, 1.7f, 0.4f, "1/4"},
        {"Lo-Fi Charm", 0.6f, 0.75f, 2.0f, -1.0f, "bitcrusher", 0.35f, 0.45f, 0.4f, true, "lowpass", 2400.0f, 0.4f, 0.3f, "1/8"},
        {"Synth Destroyer", 0.85f, 0.9f, 5.0f, -2.5f, "foldback", 0.18f, 0.65f, 0.55f, true, "bandpass", 900.0f, 4.0f, 0.7f, "1/4"},
        {"Vocal Enhancer", 0.2f, 0.3f, 1.5f, 0.0f, "soft_clip", 0.22f, 0.3f, 0.25f, true, "highpass", 300.0f, 0.3f, 0.0f, "1/4"},
        {"Guitar Sizzle", 0.55f, 0.7f, 3.0f, -1.0f, "waveshaper", 0.4f, 0.5f, 0.35f, false, "bandpass", 1600.0f, 1.8f, 0.2f, "1/4"},
        {"Drum Cruncher", 0.45f, 0.6f, 2.5f, -0.5f, "hard_clip", 0.12f, 0.2f, 0.15f, false, "lowpass", 4000.0f, 0.9f, 0.5f, "1/8"}};

    // Reference to processors
    auto &distortion = processorRef.getDistortionProcessor();
    auto &delay = processorRef.getDelayProcessor();
    auto &filter = processorRef.getFilterProcessor();
    auto &pulse = processorRef.getPulseProcessor();

    // Create each preset
    for (const auto &preset : presets)
    {
        // Make sure distortion parameters are set first
        distortion.setAlgorithm(preset.algorithm); // Set algorithm first
        distortion.setDrive(preset.drive);
        distortion.setMix(preset.mix);
        distortion.setInputGain(preset.inputGain);
        distortion.setOutputGain(preset.outputGain);

        // Set delay parameters
        delay.setDelayTime(preset.delayTime);
        delay.setFeedback(preset.feedback);
        delay.setMix(preset.delayMix);
        delay.setPingPong(preset.pingPong);

        // Set filter parameters
        filter.setFilterType(preset.filterType);
        filter.setFrequency(preset.frequency);
        filter.setResonance(preset.resonance);

        // Set pulse parameters
        pulse.setMix(preset.pulseMix);
        pulse.setRate(preset.pulseRate);

        // Create an XML element for this preset
        auto presetXml = std::make_unique<juce::XmlElement>("OxidePreset");
        presetXml->setAttribute("name", preset.name);
        presetXml->setAttribute("version", "1.0");

        // Save the current state to XML
        saveProcessorStateToXml(presetXml.get());

        // Save the preset file
        juce::File presetFile = presetsDirectory.getChildFile(preset.name + ".xml");
        presetXml->writeToFile(presetFile, "");

        // Debug output
        juce::Logger::writeToLog("Created preset: " + preset.name);
    }

    // Restore original state
    loadProcessorStateFromXml(currentState.get());
}