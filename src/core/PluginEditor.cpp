#include "PluginEditor.h"

OxideAudioProcessorEditor::OxideAudioProcessorEditor(OxideAudioProcessor &p)
    : AudioProcessorEditor(&p),
      audioProcessor(p),
      layoutView(p.getDistortionProcessor(), p.getDelayProcessor(), p.getFilterProcessor(), p.getPulseProcessor())
{
    addAndMakeVisible(background);
    addAndMakeVisible(layoutView);

    // Initialize the layout view with the output buffer for oscilloscope
    layoutView.updateBuffer(p.getOutputBuffer());

    // Set up preset selection callback
    layoutView.onPresetSelected = [this](const juce::String &presetName)
    {
        if (presetName == "light_drive")
        {
            audioProcessor.getDistortionProcessor().setDrive(0.3f);
            audioProcessor.getDistortionProcessor().setMix(0.5f);
            audioProcessor.getDistortionProcessor().setInputGain(0.0f);
            audioProcessor.getDistortionProcessor().setOutputGain(0.0f);
            audioProcessor.getDistortionProcessor().setAlgorithm(DistortionAlgorithm::SoftClip);

            // Add filter preset values
            audioProcessor.getFilterProcessor().setFilterType(FilterType::LowPass);
            audioProcessor.getFilterProcessor().setFrequency(1200.0f);
            audioProcessor.getFilterProcessor().setResonance(0.5f);

            // Add pulse preset values
            audioProcessor.getPulseProcessor().setMix(0.2f);
            audioProcessor.getPulseProcessor().setRate(Rate::Quarter);
        }
        else if (presetName == "heavy_metal")
        {
            audioProcessor.getDistortionProcessor().setDrive(0.8f);
            audioProcessor.getDistortionProcessor().setMix(0.7f);
            audioProcessor.getDistortionProcessor().setInputGain(3.0f);
            audioProcessor.getDistortionProcessor().setOutputGain(2.0f);
            audioProcessor.getDistortionProcessor().setAlgorithm(DistortionAlgorithm::HardClip);

            // Add filter preset values
            audioProcessor.getFilterProcessor().setFilterType(FilterType::LowPass);
            audioProcessor.getFilterProcessor().setFrequency(2000.0f);
            audioProcessor.getFilterProcessor().setResonance(1.2f);

            // Add pulse preset values
            audioProcessor.getPulseProcessor().setMix(0.4f);
            audioProcessor.getPulseProcessor().setRate(Rate::Quarter);
        }
        else if (presetName == "fuzz")
        {
            audioProcessor.getDistortionProcessor().setDrive(1.0f);
            audioProcessor.getDistortionProcessor().setMix(1.0f);
            audioProcessor.getDistortionProcessor().setInputGain(6.0f);
            audioProcessor.getDistortionProcessor().setOutputGain(-3.0f);
            audioProcessor.getDistortionProcessor().setAlgorithm(DistortionAlgorithm::Foldback);

            // Add filter preset values
            audioProcessor.getFilterProcessor().setFilterType(FilterType::HighPass);
            audioProcessor.getFilterProcessor().setFrequency(500.0f);
            audioProcessor.getFilterProcessor().setResonance(0.8f);

            // Add pulse preset values
            audioProcessor.getPulseProcessor().setMix(1.0f);
            audioProcessor.getPulseProcessor().setRate(Rate::Quarter);
        }
        else if (presetName == "warm_tape")
        {
            audioProcessor.getDistortionProcessor().setDrive(0.5f);
            audioProcessor.getDistortionProcessor().setMix(0.4f);
            audioProcessor.getDistortionProcessor().setInputGain(1.5f);
            audioProcessor.getDistortionProcessor().setOutputGain(0.0f);
            audioProcessor.getDistortionProcessor().setAlgorithm(DistortionAlgorithm::Waveshaper);

            // Add filter preset values
            audioProcessor.getFilterProcessor().setFilterType(FilterType::BandPass);
            audioProcessor.getFilterProcessor().setFrequency(1500.0f);
            audioProcessor.getFilterProcessor().setResonance(2.0f);

            // Add pulse preset values
            audioProcessor.getPulseProcessor().setMix(0.6f);
            audioProcessor.getPulseProcessor().setRate(Rate::Quarter);
        }
        else // default preset
        {
            audioProcessor.getDistortionProcessor().setDrive(0.5f);
            audioProcessor.getDistortionProcessor().setMix(0.5f);
            audioProcessor.getDistortionProcessor().setInputGain(0.0f);
            audioProcessor.getDistortionProcessor().setOutputGain(0.0f);
            audioProcessor.getDistortionProcessor().setAlgorithm(DistortionAlgorithm::SoftClip);

            // Add filter preset values
            audioProcessor.getFilterProcessor().setFilterType(FilterType::LowPass);
            audioProcessor.getFilterProcessor().setFrequency(1000.0f);
            audioProcessor.getFilterProcessor().setResonance(0.7f);

            // Add pulse preset values
            audioProcessor.getPulseProcessor().setMix(0.0f);
            audioProcessor.getPulseProcessor().setRate(Rate::Quarter);
        }

        // Update the UI to reflect the new values
        layoutView.setInputGain(audioProcessor.getDistortionProcessor().getInputGain());
        layoutView.setOutputGain(audioProcessor.getDistortionProcessor().getOutputGain());
    };

    layoutView.onSaveClicked = [this]()
    {
        // Here you would implement saving the current settings
        // For example, show a file save dialog and save the current state
    };

    // Set up callbacks for input/output gain changes from the UI
    layoutView.onInputGainChanged = [this](float newGain)
    {
        audioProcessor.getDistortionProcessor().setInputGain(newGain);
    };

    layoutView.onOutputGainChanged = [this](float newGain)
    {
        audioProcessor.getDistortionProcessor().setOutputGain(newGain);
    };

    // Initialize with the current gain values
    layoutView.setInputGain(audioProcessor.getDistortionProcessor().getInputGain());
    layoutView.setOutputGain(audioProcessor.getDistortionProcessor().getOutputGain());

    // Start the timer for meter updates
    startTimerHz(30);

    // Set initial size
    setSize(CANVAS_WIDTH, CANVAS_HEIGHT);
}

OxideAudioProcessorEditor::~OxideAudioProcessorEditor()
{
    stopTimer();
}

void OxideAudioProcessorEditor::paint(juce::Graphics &g)
{
    // No need to paint anything here since the background component does that
}

void OxideAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    // Set the background to cover the entire window
    background.setBounds(bounds);

    // Set the layout view to cover the entire window as well
    layoutView.setBounds(bounds);
}

void OxideAudioProcessorEditor::timerCallback()
{
    float leftLevel = audioProcessor.getLeftLevel();
    float rightLevel = audioProcessor.getRightLevel();

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

    // Allow small values to show as completely empty
    if (leftLevel < 0.1f)
        leftLevel = 0.0f;
    if (rightLevel < 0.1f)
        rightLevel = 0.0f;

    // Update the levels in the layout view
    layoutView.updateLevels(leftLevel, rightLevel);

    // Update the oscilloscope with latest audio buffer
    layoutView.updateBuffer(audioProcessor.getOutputBuffer());
}