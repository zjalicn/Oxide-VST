#pragma once

#include <JuceHeader.h>
#include "MeterView.h"
#include "ControlPanel.h"
#include "DistortionProcessor.h"

class MainComponent : public juce::AudioAppComponent,
                      private juce::Timer
{
public:
    MainComponent();
    ~MainComponent() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    DistortionProcessor distortionProcessor;
    MeterView meterView;
    ControlPanel controlPanel;
    
    juce::LinearSmoothedValue<float> levelLeft, levelRight;
    
    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};