#include "MainComponent.h"

MainComponent::MainComponent()
    : controlPanel(distortionProcessor)
{
    setAudioChannels(2, 2);
    
    addAndMakeVisible(meterView);
    addAndMakeVisible(controlPanel);
    
    startTimerHz(30);
    
    setSize(800, 600);
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    levelLeft.reset(sampleRate, 0.1);  // Smooth over 100ms
    levelRight.reset(sampleRate, 0.1);
    
    distortionProcessor.prepare(sampleRate);
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    auto* inputBuffer = bufferToFill.buffer;
    
    float newLevelLeft = 0.0f;
    float newLevelRight = 0.0f;
    
    if (inputBuffer->getNumChannels() > 0)
        newLevelLeft = inputBuffer->getRMSLevel(0, bufferToFill.startSample, bufferToFill.numSamples);
    
    if (inputBuffer->getNumChannels() > 1)
        newLevelRight = inputBuffer->getRMSLevel(1, bufferToFill.startSample, bufferToFill.numSamples);
    
    levelLeft.skip(bufferToFill.numSamples);
    levelRight.skip(bufferToFill.numSamples);
    
    levelLeft.applyGain(newLevelLeft);
    levelRight.applyGain(newLevelRight);
    
    distortionProcessor.processBlock(*bufferToFill.buffer);
}

void MainComponent::releaseResources()
{
    // Free any resources when shutting down
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff222222));
}

void MainComponent::resized()
{
    auto bounds = getLocalBounds();
    
    // Layout: meters on top, controls on bottom
    auto controlHeight = 150;
    
    controlPanel.setBounds(bounds.removeFromBottom(controlHeight));
    meterView.setBounds(bounds);
}

void MainComponent::timerCallback()
{
    float leftLevel = juce::jlimit(0.0f, 100.0f, levelLeft.getCurrentValue() * 100.0f);
    float rightLevel = juce::jlimit(0.0f, 100.0f, levelRight.getCurrentValue() * 100.0f);
    
    // Apply some scaling to make the meters more responsive
    leftLevel = std::pow(leftLevel, 0.5f) * 10.0f;
    rightLevel = std::pow(rightLevel, 0.5f) * 10.0f;
    
    leftLevel = juce::jlimit(0.0f, 100.0f, leftLevel);
    rightLevel = juce::jlimit(0.0f, 100.0f, rightLevel);
    
    // Update the meters
    meterView.updateLevels(leftLevel, rightLevel);
}