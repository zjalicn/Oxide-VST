#include "PluginEditor.h"

OxideAudioProcessorEditor::OxideAudioProcessorEditor(OxideAudioProcessor& p)
    : AudioProcessorEditor(&p), 
      audioProcessor(p),
      controlPanel(p.getDistortionProcessor())
{
    addAndMakeVisible(meterView);
    addAndMakeVisible(controlPanel);
    
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
    g.fillAll(juce::Colour(0xff222222));
}

void OxideAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    
    auto controlHeight = 150;
    
    controlPanel.setBounds(bounds.removeFromBottom(controlHeight));
    meterView.setBounds(bounds);
}

void OxideAudioProcessorEditor::timerCallback()
{
    float leftLevel = audioProcessor.getLeftLevel();
    float rightLevel = audioProcessor.getRightLevel();
    
    leftLevel = std::pow(leftLevel * 100.0f, 0.5f) * 10.0f;
    rightLevel = std::pow(rightLevel * 100.0f, 0.5f) * 10.0f;
    
    leftLevel = juce::jlimit(0.0f, 100.0f, leftLevel);
    rightLevel = juce::jlimit(0.0f, 100.0f, rightLevel);
    
    meterView.updateLevels(leftLevel, rightLevel);
}