#pragma once
#include <JuceHeader.h>
#include "DistortionAudioProcessor.h"

class WaveformVisualizer : public juce::Component,
                          public juce::Timer
{
public:
    WaveformVisualizer(DistortionAudioProcessor&);
    ~WaveformVisualizer() override;
    
    void paint(juce::Graphics&) override;
    void timerCallback() override;
    
private:
    DistortionAudioProcessor& processor;
    std::vector<float> waveformPoints;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformVisualizer)
};

class SpectrumAnalyzer : public juce::Component,
                        public juce::Timer
{
public:
    SpectrumAnalyzer(DistortionAudioProcessor&);
    ~SpectrumAnalyzer() override;
    
    void paint(juce::Graphics&) override;
    void timerCallback() override;
    
private:
    DistortionAudioProcessor& processor;
    std::vector<float> spectrum;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAnalyzer)
};

class DistortionAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    DistortionAudioProcessorEditor(DistortionAudioProcessor&);
    ~DistortionAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    DistortionAudioProcessor& processor;
    
    WaveformVisualizer waveformVisualizer;
    SpectrumAnalyzer spectrumAnalyzer;
    
    juce::Slider driveSlider;
    juce::Slider mixSlider;
    juce::Slider outputSlider;
    juce::ComboBox typeBox;
    juce::ComboBox oversamplingBox;
    
    juce::Slider lfoRateSlider;
    juce::Slider lfoDepthSlider;
    juce::Slider envFollowerSlider;
    
    juce::Slider preFilterFreqSlider;
    juce::Slider preFilterResSlider;
    juce::Slider postFilterFreqSlider;
    juce::Slider postFilterResSlider;
    
    juce::Slider stereoWidthSlider;
    
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sliderAttachments;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> comboBoxAttachments;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DistortionAudioProcessorEditor)
};
