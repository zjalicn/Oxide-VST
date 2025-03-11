#pragma once

#include <JuceHeader.h>
#include "dsp/distortion/DistortionProcessor.h"
#include "dsp/delay/DelayProcessor.h"
#include "dsp/filter/FilterProcessor.h"
#include "dsp/pulse/PulseProcessor.h"

// Forward declare PresetManager
class PresetManager;

class OxideAudioProcessor : public juce::AudioProcessor
{
public:
    OxideAudioProcessor();
    ~OxideAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout &layouts) const override;

    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

    juce::AudioProcessorEditor *createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String &newName) override;

    void getStateInformation(juce::MemoryBlock &destData) override;
    void setStateInformation(const void *data, int sizeInBytes) override;

    // Accessors for DSP processors
    DistortionProcessor &getDistortionProcessor() { return distortionProcessor; }
    DelayProcessor &getDelayProcessor() { return delayProcessor; }
    FilterProcessor &getFilterProcessor() { return filterProcessor; }
    PulseProcessor &getPulseProcessor() { return pulseProcessor; }

    // Accessor for PresetManager - might return nullptr if not initialized yet
    PresetManager *getPresetManager();

    float getLeftLevel() const { return levelLeft.getCurrentValue(); }
    float getRightLevel() const { return levelRight.getCurrentValue(); }

    juce::AudioBuffer<float> getOutputBuffer()
    {
        juce::ScopedLock lock(outputBufferLock);
        return outputBuffer;
    }

private:
    // Initialize DSP processors first
    DelayProcessor delayProcessor;
    DistortionProcessor distortionProcessor;
    FilterProcessor filterProcessor;
    PulseProcessor pulseProcessor;

    // Initialize PresetManager after all processors are set up
    std::unique_ptr<PresetManager> presetManager;
    bool presetManagerInitialized = false;

    juce::LinearSmoothedValue<float> levelLeft, levelRight;

    // Buffer for oscilloscope
    juce::AudioBuffer<float> outputBuffer;
    juce::CriticalSection outputBufferLock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OxideAudioProcessor)
};