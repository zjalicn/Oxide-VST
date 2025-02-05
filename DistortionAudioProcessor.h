#pragma once
#include <JuceHeader.h>

class DistortionAudioProcessor : public juce::AudioProcessor
{
public:
    enum DistortionType { SoftClip, HardClip, Foldback, Waveshaper, Bitcrusher };
    enum OversamplingFactor { None, Factor2x, Factor4x, Factor8x };

    DistortionAudioProcessor();
    ~DistortionAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    float getCurrentLFOValue() const { return currentLFOValue; }
    float getCurrentEnvValue() const { return envelopeFollower.getCurrentValue(); }
    const juce::dsp::FFT& getFFT() const { return fft; }

private:
    // Parameters
    juce::AudioProcessorValueTreeState apvts;
    std::atomic<float>* typeParam = nullptr;
    std::atomic<float>* driveParam = nullptr;
    std::atomic<float>* mixParam = nullptr;
    std::atomic<float>* outputParam = nullptr;
    std::atomic<float>* stereoWidthParam = nullptr;
    std::atomic<float>* oversamplingParam = nullptr;
    
    // Modulation
    std::atomic<float>* lfoRateParam = nullptr;
    std::atomic<float>* lfoDepthParam = nullptr;
    std::atomic<float>* envFollowerParam = nullptr;
    float currentLFOValue = 0.0f;
    float lfoPhase = 0.0f;
    
    // Filters
    std::atomic<float>* preFilterFreqParam = nullptr;
    std::atomic<float>* preFilterResParam = nullptr;
    std::atomic<float>* postFilterFreqParam = nullptr;
    std::atomic<float>* postFilterResParam = nullptr;
    
    // DSP Objects
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampling;
    juce::dsp::IIR::Filter<float> preFilter, postFilter;
    juce::dsp::FFT fft;
    class EnvelopeFollower
    {
    public:
        void prepare(double sampleRate);
        void process(float input);
        float getCurrentValue() const { return currentValue; }
    private:
        float currentValue = 0.0f;
        float attack = 0.1f;
        float release = 0.1f;
        double sampleRate = 44100.0;
    } envelopeFollower;

    float processDistortion(float input, DistortionType type, float drive);
    void updateFilters();
    void updateLFO(double sampleRate);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DistortionAudioProcessor)
};
