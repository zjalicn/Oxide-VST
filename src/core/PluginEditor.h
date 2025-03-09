#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ControlPanel.h"
#include "DelayControlPanel.h"
#include "MeterView.h"
#include "HeaderView.h"
#include "Background.h"
#include "OscilloscopeView.h"

class OxideAudioProcessorEditor : public juce::AudioProcessorEditor,
                                  private juce::Timer
{
public:
    OxideAudioProcessorEditor(OxideAudioProcessor &);
    ~OxideAudioProcessorEditor() override;

    void paint(juce::Graphics &) override;
    void resized() override;

private:
    OxideAudioProcessor &audioProcessor;

    Background background;
    HeaderView headerView;
    MeterView meterView;
    ControlPanel controlPanel;
    DelayControlPanel delayControlPanel;
    OscilloscopeView oscilloscopeView;

    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OxideAudioProcessorEditor)
};