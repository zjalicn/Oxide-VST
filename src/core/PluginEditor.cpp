#include "PluginEditor.h"
#include "PresetManager.h"

OxideAudioProcessorEditor::OxideAudioProcessorEditor(OxideAudioProcessor &p)
    : AudioProcessorEditor(&p),
      audioProcessor(p),
      layoutView(p.getDistortionProcessor(), p.getDelayProcessor(), p.getFilterProcessor(), p.getPulseProcessor()),
      presetLoadRefreshCounter(0)
{
    addAndMakeVisible(background);
    addAndMakeVisible(layoutView);

    // Initialize the layout view with the output buffer for oscilloscope
    layoutView.updateBuffer(p.getOutputBuffer());

    // Get the PresetManager safely
    auto *presetManager = audioProcessor.getPresetManager();

    // Set up preset selection callback
    layoutView.onPresetSelected = [this](const juce::String &presetName)
    {
        // Get the PresetManager safely
        auto *presetManager = audioProcessor.getPresetManager();
        if (presetManager && presetManager->loadPreset(presetName))
        {
            updateUIAfterPresetLoad();

            // Start a separate timer for multiple refresh attempts after preset loading
            presetLoadRefreshCounter = 0;
            startTimer(50); // Refresh UI after a short delay
        }
    };

    // Update the preset dropdown with available presets
    if (presetManager != nullptr)
    {
        layoutView.updatePresetList(presetManager->getPresetList());
    }
    else
    {
        // Provide a default preset list if PresetManager isn't available
        juce::StringArray defaultList;
        defaultList.add("Default");
        layoutView.updatePresetList(defaultList);
    }

    layoutView.onSaveClicked = [this]()
    {
        // Get the PresetManager safely
        auto *presetManager = audioProcessor.getPresetManager();
        if (presetManager == nullptr)
        {
            return;
        }

        // Use asynchronous alert window
        auto saveDialog = std::make_unique<juce::AlertWindow>(
            "Save Preset", "Enter a name for this preset:", juce::AlertWindow::QuestionIcon);

        saveDialog->addTextEditor("presetName", "New Preset", "Preset Name:");
        saveDialog->addButton("Save", 1, juce::KeyPress(juce::KeyPress::returnKey));
        saveDialog->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

        // Use lambdas carefully to avoid capturing this pointer across threads
        auto *pm = presetManager;
        auto &lv = layoutView;

        saveDialog->enterModalState(true, juce::ModalCallbackFunction::create(
                                              [pm, &lv, saveDialogPtr = saveDialog.release()](int result)
                                              {
                                                  std::unique_ptr<juce::AlertWindow> ownedDialog(saveDialogPtr);

                                                  if (result == 1)
                                                  {
                                                      juce::String presetName = ownedDialog->getTextEditorContents("presetName");
                                                      if (presetName.isNotEmpty())
                                                      {
                                                          pm->savePreset(presetName);

                                                          // Update the preset dropdown with the new list
                                                          lv.updatePresetList(pm->getPresetList());
                                                      }
                                                  }
                                              }));
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

void OxideAudioProcessorEditor::updateUIAfterPresetLoad()
{
    // Update the input/output gain display
    layoutView.setInputGain(audioProcessor.getDistortionProcessor().getInputGain());
    layoutView.setOutputGain(audioProcessor.getDistortionProcessor().getOutputGain());

    // Force a refresh of all UI parameters
    layoutView.refreshAllParameters();
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
    // If in preset loading mode, do additional refreshes
    if (presetLoadRefreshCounter >= 0 && presetLoadRefreshCounter < 5)
    {
        // Force multiple UI refreshes after preset loading
        layoutView.refreshAllParameters();
        presetLoadRefreshCounter++;

        if (presetLoadRefreshCounter >= 5)
        {
            // Reset counter and return to normal update frequency
            presetLoadRefreshCounter = -1;
            stopTimer();
            startTimerHz(30);
        }

        // Skip the regular updates during preset loading refresh cycle
        return;
    }

    // Regular meter updates
    float leftLevel = audioProcessor.getLeftLevel();
    float rightLevel = audioProcessor.getRightLevel();
    float outLeftLevel = audioProcessor.getOutputLeftLevel();
    float outRightLevel = audioProcessor.getOutputRightLevel();

    // Apply input gain to the displayed levels to match the actual processing
    float inputGainLinear = std::pow(10.0f, audioProcessor.getDistortionProcessor().getInputGain() / 20.0f);
    leftLevel *= inputGainLinear;
    rightLevel *= inputGainLinear;

    // Scale for display - convert RMS values to percentage heights
    leftLevel = std::pow(leftLevel * 100.0f, 0.5f) * 10.0f;
    rightLevel = std::pow(rightLevel * 100.0f, 0.5f) * 10.0f;
    outLeftLevel = std::pow(outLeftLevel * 100.0f, 0.5f) * 10.0f;
    outRightLevel = std::pow(outRightLevel * 100.0f, 0.5f) * 10.0f;

    // Ensure values are in range
    leftLevel = juce::jlimit(0.0f, 100.0f, leftLevel);
    rightLevel = juce::jlimit(0.0f, 100.0f, rightLevel);
    outLeftLevel = juce::jlimit(0.0f, 100.0f, outLeftLevel);
    outRightLevel = juce::jlimit(0.0f, 100.0f, outRightLevel);

    // Allow small values to show as completely empty
    if (leftLevel < 0.1f)
        leftLevel = 0.0f;
    if (rightLevel < 0.1f)
        rightLevel = 0.0f;
    if (outLeftLevel < 0.1f)
        outLeftLevel = 0.0f;
    if (outRightLevel < 0.1f)
        outRightLevel = 0.0f;

    // Update the levels in the layout view
    layoutView.updateLevels(leftLevel, rightLevel, outLeftLevel, outRightLevel);

    // Update the oscilloscope with latest audio buffer
    layoutView.updateBuffer(audioProcessor.getOutputBuffer());
}