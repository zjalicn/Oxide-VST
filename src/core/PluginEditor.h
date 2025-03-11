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

    // Counter for multiple UI refreshes after preset loading
    int presetLoadRefreshCounter = -1;

    void timerCallback() override;
    void updateUIAfterPresetLoad();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OxideAudioProcessorEditor)
};