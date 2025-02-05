#include "DistortionAudioProcessorEditor.h"

DistortionAudioProcessorEditor::DistortionAudioProcessorEditor(DistortionAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p),
      waveformVisualizer(p), spectrumAnalyzer(p)
{
    // Configure visualizers
    addAndMakeVisible(waveformVisualizer);
    addAndMakeVisible(spectrumAnalyzer);

    // Main controls
    addAndMakeVisible(driveSlider);
    driveSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    driveSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    
    addAndMakeVisible(typeBox);
    typeBox.addItemList({"Soft Clip", "Hard Clip", "Foldback", "Waveshaper", "Bitcrusher"}, 1);
    
    addAndMakeVisible(oversamplingBox);
    oversamplingBox.addItemList({"None", "2x", "4x", "8x"}, 1);

    // Modulation
    addAndMakeVisible(lfoRateSlider);
    lfoRateSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    
    addAndMakeVisible(lfoDepthSlider);
    lfoDepthSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    
    addAndMakeVisible(envFollowerSlider);
    envFollowerSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);

    // Filters
    addAndMakeVisible(preFilterFreqSlider);
    preFilterFreqSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    
    addAndMakeVisible(preFilterResSlider);
    preFilterResSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    
    addAndMakeVisible(postFilterFreqSlider);
    postFilterFreqSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    
    addAndMakeVisible(postFilterResSlider);
    postFilterResSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);

    // Stereo
    addAndMakeVisible(stereoWidthSlider);
    stereoWidthSlider.setSliderStyle(juce::Slider::LinearHorizontal);

    // Parameter attachments
    auto& apvts = processor.getAPVTS();
    
    sliderAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "drive", driveSlider));
    sliderAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "mix", mixSlider));
    sliderAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "output", outputSlider));
    sliderAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "lfoRate", lfoRateSlider));
    sliderAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "lfoDepth", lfoDepthSlider));
    sliderAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "envFollower", envFollowerSlider));
    sliderAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "preFilterFreq", preFilterFreqSlider));
    sliderAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "preFilterRes", preFilterResSlider));
    sliderAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "postFilterFreq", postFilterFreqSlider));
    sliderAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "postFilterRes", postFilterResSlider));
    sliderAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "width", stereoWidthSlider));

    comboBoxAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, "type", typeBox));
    comboBoxAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, "oversampling", oversamplingBox));

    setSize(800, 600);
}

void DistortionAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
}

void DistortionAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    
    // Visualizers at top
    auto visArea = bounds.removeFromTop(200);
    waveformVisualizer.setBounds(visArea.removeFromLeft(visArea.getWidth() / 2));
    spectrumAnalyzer.setBounds(visArea);
    
    // Control section
    auto controlArea = bounds;
    
    // Main controls
    auto mainControls = controlArea.removeFromLeft(200);
    typeBox.setBounds(mainControls.removeFromTop(30));
    oversamplingBox.setBounds(mainControls.removeFromTop(30));
    driveSlider.setBounds(mainControls.removeFromTop(100));
    mixSlider.setBounds(mainControls.removeFromTop(100));
    outputSlider.setBounds(mainControls.removeFromTop(100));
    
    // Modulation
    auto modControls = controlArea.removeFromLeft(200);
    lfoRateSlider.setBounds(modControls.removeFromTop(100));
    lfoDepthSlider.setBounds(modControls.removeFromTop(100));
    envFollowerSlider.setBounds(modControls.removeFromTop(100));
    
    // Filters
    auto filterControls = controlArea.removeFromLeft(200);
    preFilterFreqSlider.setBounds(filterControls.removeFromTop(100));
    preFilterResSlider.setBounds(filterControls.removeFromTop(100));
    postFilterFreqSlider.setBounds(filterControls.removeFromTop(100));
    postFilterResSlider.setBounds(filterControls.removeFromTop(100));
    
    // Stereo width at bottom
    stereoWidthSlider.setBounds(bounds.removeFromBottom(30));
}

// WaveformVisualizer Implementation
WaveformVisualizer::WaveformVisualizer(DistortionAudioProcessor& p) : processor(p)
{
    startTimerHz(30);
}

WaveformVisualizer::~WaveformVisualizer()
{
    stopTimer();
}

void WaveformVisualizer::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::green);
    
    auto bounds = getLocalBounds().toFloat();
    auto centerY = bounds.getCentreY();
    juce::Path waveform;
    
    const float width = bounds.getWidth();
    
    if (!waveformPoints.empty())
    {
        waveform.startNewSubPath(0, centerY + waveformPoints[0] * bounds.getHeight() * 0.5f);
        
        for (int i = 1; i < waveformPoints.size(); ++i)
        {
            float x = (i / static_cast<float>(waveformPoints.size())) * width;
            float y = centerY + waveformPoints[i] * bounds.getHeight() * 0.5f;
            waveform.lineTo(x, y);
        }
    }
    
    g.strokePath(waveform, juce::PathStrokeType(2.0f));
}

void WaveformVisualizer::timerCallback()
{
    // Update waveform data
    repaint();
}

// SpectrumAnalyzer Implementation
SpectrumAnalyzer::SpectrumAnalyzer(DistortionAudioProcessor& p) : processor(p)
{
    startTimerHz(30);
}

SpectrumAnalyzer::~SpectrumAnalyzer()
{
    stopTimer();
}

void SpectrumAnalyzer::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::yellow);
    
    auto bounds = getLocalBounds().toFloat();
    auto width = bounds.getWidth();
    auto height = bounds.getHeight();
    
    if (!spectrum.empty())
    {
        juce::Path spectrumPath;
        spectrumPath.startNewSubPath(0, height);
        
        for (int i = 0; i < spectrum.size(); ++i)
        {
            float x = std::log10(1 + i) / std::log10(spectrum.size()) * width;
            float y = height - (spectrum[i] * height);
            spectrumPath.lineTo(x, y);
        }
        
        spectrumPath.lineTo(width, height);
        spectrumPath.closeSubPath();
        
        g.fillPath(spectrumPath);
    }
}

void SpectrumAnalyzer::timerCallback()
{
    // Update spectrum data
    repaint();
}
