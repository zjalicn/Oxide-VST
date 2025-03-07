#include "ControlPanel.h"

ControlPanel::ControlPanel(DistortionProcessor& processor)
    : distortionProcessor(processor)
{
    // Set up the Drive slider
    driveSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    driveSlider.setRange(0.0, 1.0, 0.01);
    driveSlider.setValue(distortionProcessor.getDrive());
    driveSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    driveSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::orange);
    driveSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    driveSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::darkgrey);
    driveSlider.setColour(juce::Slider::thumbColourId, juce::Colours::orange);
    driveSlider.addListener(this);
    addAndMakeVisible(driveSlider);
    
    // Set up the Mix slider
    mixSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    mixSlider.setRange(0.0, 1.0, 0.01);
    mixSlider.setValue(distortionProcessor.getMix());
    mixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    mixSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::orange);
    mixSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    mixSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::darkgrey);
    mixSlider.setColour(juce::Slider::thumbColourId, juce::Colours::orange);
    mixSlider.addListener(this);
    addAndMakeVisible(mixSlider);
    
    // Set up labels
    driveLabel.setText("Drive", juce::dontSendNotification);
    driveLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    driveLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(driveLabel);
    
    mixLabel.setText("Mix", juce::dontSendNotification);
    mixLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    mixLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(mixLabel);
}

ControlPanel::~ControlPanel()
{
}

void ControlPanel::paint(juce::Graphics& g)
{
    // Fill the background
    g.fillAll(juce::Colour(0xff222222));
    
    // Draw a border
    g.setColour(juce::Colours::darkgrey);
    g.drawRect(getLocalBounds());
    
    // Draw a title
    g.setColour(juce::Colours::orange);
    g.setFont(20.0f);
    g.drawText("Distortion", getLocalBounds().removeFromTop(30), juce::Justification::centred, true);
}

void ControlPanel::resized()
{
    auto bounds = getLocalBounds().reduced(10);
    
    // Title area
    bounds.removeFromTop(30);
    
    // Layout for controls
    auto controlArea = bounds;
    
    // Drive section
    auto driveArea = controlArea.removeFromLeft(controlArea.getWidth() / 2);
    driveLabel.setBounds(driveArea.removeFromTop(20));
    driveSlider.setBounds(driveArea.reduced(10));
    
    // Mix section
    mixLabel.setBounds(controlArea.removeFromTop(20));
    mixSlider.setBounds(controlArea.reduced(10));
}

void ControlPanel::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &driveSlider)
    {
        distortionProcessor.setDrive((float)driveSlider.getValue());
    }
    else if (slider == &mixSlider)
    {
        distortionProcessor.setMix((float)mixSlider.getValue());
    }
}