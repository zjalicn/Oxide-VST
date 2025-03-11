#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "LayoutView.h"
#include "Background.h"

class OxideAudioProcessorEditor : public juce::AudioProcessorEditor,
                                  private juce::Timer
{
public:
    static constexpr int CANVAS_WIDTH = 720;
    static constexpr int CANVAS_HEIGHT = 320;

    OxideAudioProcessorEditor(OxideAudioProcessor &);
    ~OxideAudioProcessorEditor() override;

    void paint(juce::Graphics &g) override;
    void resized() override;

private:
    OxideAudioProcessor &audioProcessor;

    Background background;
    LayoutView layoutView;

    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OxideAudioProcessorEditor)
};