#include "PluginEditor.h"

OxideAudioProcessorEditor::OxideAudioProcessorEditor(OxideAudioProcessor& p)
    : AudioProcessorEditor(&p), 
      audioProcessor(p),
      controlPanel(p.getDistortionProcessor())
{
    // Add background first so it's behind everything
    addAndMakeVisible(background);
    
    // Add all other components
    addAndMakeVisible(headerView);
    addAndMakeVisible(meterView);
    addAndMakeVisible(controlPanel);
    
    // Set up callbacks for header actions
    headerView.onPresetSelected = [this](const juce::String& presetName) {
        // Handle preset selection
        juce::Logger::writeToLog("Preset selected: " + presetName);
        
        // For example, you could load preset values:
        if (presetName == "light_drive") {
            audioProcessor.getDistortionProcessor().setDrive(0.3f);
            audioProcessor.getDistortionProcessor().setMix(0.5f);
            audioProcessor.getDistortionProcessor().setInputGain(0.0f);
            audioProcessor.getDistortionProcessor().setOutputGain(0.0f);
        }
        else if (presetName == "heavy_metal") {
            audioProcessor.getDistortionProcessor().setDrive(0.8f);
            audioProcessor.getDistortionProcessor().setMix(0.7f);
            audioProcessor.getDistortionProcessor().setInputGain(3.0f);
            audioProcessor.getDistortionProcessor().setOutputGain(2.0f);
        }
        else if (presetName == "fuzz") {
            audioProcessor.getDistortionProcessor().setDrive(1.0f);
            audioProcessor.getDistortionProcessor().setMix(1.0f);
            audioProcessor.getDistortionProcessor().setInputGain(6.0f);
            audioProcessor.getDistortionProcessor().setOutputGain(-3.0f);
        }
        else if (presetName == "warm_tape") {
            audioProcessor.getDistortionProcessor().setDrive(0.5f);
            audioProcessor.getDistortionProcessor().setMix(0.4f);
            audioProcessor.getDistortionProcessor().setInputGain(1.5f);
            audioProcessor.getDistortionProcessor().setOutputGain(0.0f);
        }
        else { // default
            audioProcessor.getDistortionProcessor().setDrive(0.5f);
            audioProcessor.getDistortionProcessor().setMix(0.5f);
            audioProcessor.getDistortionProcessor().setInputGain(0.0f);
            audioProcessor.getDistortionProcessor().setOutputGain(0.0f);
        }
        
        // Update the UI to reflect the new values
        meterView.setInputGain(audioProcessor.getDistortionProcessor().getInputGain());
        meterView.setOutputGain(audioProcessor.getDistortionProcessor().getOutputGain());
    };
    
    headerView.onSaveClicked = [this]() {
        // Handle save action
        juce::Logger::writeToLog("Save clicked");
        
        // Here you would implement saving the current settings
        // For example, you could show a file save dialog
        // and save the current state as a preset file
    };
    
    // Set up callbacks for input/output gain changes from the UI
    meterView.onInputGainChanged = [this](float newGain) {
        audioProcessor.getDistortionProcessor().setInputGain(newGain);
    };
    
    meterView.onOutputGainChanged = [this](float newGain) {
        audioProcessor.getDistortionProcessor().setOutputGain(newGain);
    };
    
    // Initialize the meter with the current gain values
    meterView.setInputGain(audioProcessor.getDistortionProcessor().getInputGain());
    meterView.setOutputGain(audioProcessor.getDistortionProcessor().getOutputGain());
    
    // Start the timer for meter updates
    startTimerHz(30);
    
    // Set initial size
    setSize(800, 600);
}

OxideAudioProcessorEditor::~OxideAudioProcessorEditor()
{
    stopTimer();
}

void OxideAudioProcessorEditor::paint(juce::Graphics& g)
{
    // No need to paint anything here since the background component does that
}

void OxideAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    
    // Header height
    auto headerHeight = 60;
    
    // Header at the top
    headerView.setBounds(bounds.removeFromTop(headerHeight));
    
    // Background fills everything below the header
    background.setBounds(bounds);
    
    // Control panel height
    auto controlHeight = 150;
    
    // The remaining area for controls
    auto contentBounds = bounds;
    
    // Position the control panel at the bottom
    controlPanel.setBounds(contentBounds.removeFromBottom(controlHeight));
    
    // The meter view takes the remaining space
    meterView.setBounds(contentBounds);
}

void OxideAudioProcessorEditor::timerCallback()
{
    // Get raw levels from the processor
    float leftLevel = audioProcessor.getLeftLevel();
    float rightLevel = audioProcessor.getRightLevel();
    
    // If there's no signal, make sure we still see something for testing
    // Remove this for production if you don't want artificial signals
    if (leftLevel < 0.01f && rightLevel < 0.01f)
    {
        // Generate a slow pulsing test signal if no real signal is detected
        static float counter = 0.0f;
        counter += 0.02f;
        if (counter > juce::MathConstants<float>::twoPi)
            counter -= juce::MathConstants<float>::twoPi;
            
        // Create a mild pulsing sine wave for the meters
        float pulse = (std::sin(counter) * 0.4f + 0.6f) * 15.0f;
        leftLevel = pulse;
        rightLevel = pulse * 0.8f;
    }
    
    // Apply input gain to the displayed levels to match the actual processing
    float inputGainLinear = std::pow(10.0f, audioProcessor.getDistortionProcessor().getInputGain() / 20.0f);
    leftLevel *= inputGainLinear;
    rightLevel *= inputGainLinear;
    
    // Scale for display - convert RMS values to percentage heights
    leftLevel = std::pow(leftLevel * 100.0f, 0.5f) * 10.0f; 
    rightLevel = std::pow(rightLevel * 100.0f, 0.5f) * 10.0f;
    
    // Ensure values are in range
    leftLevel = juce::jlimit(0.0f, 100.0f, leftLevel);
    rightLevel = juce::jlimit(0.0f, 100.0f, rightLevel);
    
    // Make sure we have visible meter movement
    if (leftLevel < 5.0f) leftLevel = 5.0f;
    if (rightLevel < 5.0f) rightLevel = 5.0f;
    
    // Update the meter display
    meterView.updateLevels(leftLevel, rightLevel);
}