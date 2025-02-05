#pragma once
#include <JuceHeader.h>

inline juce::AudioProcessorValueTreeState::ParameterLayout createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Distortion parameters
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "type", "Type",
        juce::StringArray{"Soft Clip", "Hard Clip", "Foldback", "Waveshaper", "Bitcrusher"},
        0));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "drive", "Drive",
        juce::NormalisableRange<float>(1.0f, 25.0f, 0.1f, 0.5f),
        1.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "mix", "Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        1.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "output", "Output",
        juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f, 0.5f),
        1.0f));

    // Oversampling
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "oversampling", "Oversampling",
        juce::StringArray{"None", "2x", "4x", "8x"},
        0));

    // Modulation
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "lfoRate", "LFO Rate",
        juce::NormalisableRange<float>(0.1f, 10.0f, 0.1f, 0.3f),
        1.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "lfoDepth", "LFO Depth",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "envFollower", "Env Follow",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.0f));

    // Filters
    auto freqRange = juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.25f);
    auto resRange = juce::NormalisableRange<float>(0.1f, 8.0f, 0.1f);

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "preFilterFreq", "Pre Filter Freq", freqRange, 20000.0f));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "preFilterRes", "Pre Filter Res", resRange, 0.707f));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "postFilterFreq", "Post Filter Freq", freqRange, 20000.0f));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "postFilterRes", "Post Filter Res", resRange, 0.707f));

    // Stereo
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "width", "Stereo Width",
        juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f),
        1.0f));

    return { params.begin(), params.end() };
}
