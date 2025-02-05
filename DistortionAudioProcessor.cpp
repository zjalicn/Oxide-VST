#include "DistortionAudioProcessor.h"
#include "DistortionAudioProcessorEditor.h"

DistortionAudioProcessor::DistortionAudioProcessor()
    : AudioProcessor(BusesProperties()
          .withInput("Input", juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameters()),
      fft(12) // 4096 points
{
    typeParam = apvts.getRawParameterValue("type");
    driveParam = apvts.getRawParameterValue("drive");
    mixParam = apvts.getRawParameterValue("mix");
    outputParam = apvts.getRawParameterValue("output");
    stereoWidthParam = apvts.getRawParameterValue("width");
    oversamplingParam = apvts.getRawParameterValue("oversampling");
    lfoRateParam = apvts.getRawParameterValue("lfoRate");
    lfoDepthParam = apvts.getRawParameterValue("lfoDepth");
    envFollowerParam = apvts.getRawParameterValue("envFollower");
    preFilterFreqParam = apvts.getRawParameterValue("preFilterFreq");
    preFilterResParam = apvts.getRawParameterValue("preFilterRes");
    postFilterFreqParam = apvts.getRawParameterValue("postFilterFreq");
    postFilterResParam = apvts.getRawParameterValue("postFilterRes");
}

void DistortionAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    oversampling = std::make_unique<juce::dsp::Oversampling<float>>(
        2, // num channels
        2, // max factor (8x = 2^3)
        juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR);
    
    oversampling->initProcessing(samplesPerBlock);
    
    juce::dsp::ProcessSpec spec{
        sampleRate,
        static_cast<juce::uint32>(samplesPerBlock),
        2
    };
    
    preFilter.prepare(spec);
    postFilter.prepare(spec);
    envelopeFollower.prepare(sampleRate);
    
    updateFilters();
}

void DistortionAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, 
                                          juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    
    // Update modulation
    updateLFO(getSampleRate());
    
    // Get buffer for oversampling
    const auto totalNumInputChannels = getTotalNumInputChannels();
    const auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    // Oversampling
    const auto factor = static_cast<OversamplingFactor>(static_cast<int>(*oversamplingParam));
    juce::dsp::AudioBlock<float> block(buffer);
    
    if (factor != OversamplingFactor::None)
    {
        auto osBlock = oversampling->processSamplesUp(block);
        
        // Process oversampled audio
        for (int channel = 0; channel < totalNumInputChannels; ++channel)
        {
            float* channelData = osBlock.getChannelPointer(channel);
            
            for (int sample = 0; sample < osBlock.getNumSamples(); ++sample)
            {
                // Pre-filter
                channelData[sample] = preFilter.processSample(channel, channelData[sample]);
                
                // Get modulation values
                float envMod = *envFollowerParam * envelopeFollower.getCurrentValue();
                float lfoMod = *lfoDepthParam * currentLFOValue;
                float totalDrive = *driveParam * (1.0f + envMod + lfoMod);
                
                // Process distortion
                const float input = channelData[sample];
                float distorted = processDistortion(input, 
                    static_cast<DistortionType>(static_cast<int>(*typeParam)), 
                    totalDrive);
                
                // Mix
                float mix = *mixParam;
                float processed = (1.0f - mix) * input + mix * distorted;
                
                // Post-filter
                channelData[sample] = postFilter.processSample(channel, processed);
            }
        }
        
        // Stereo width
        if (totalNumInputChannels == 2)
        {
            float width = *stereoWidthParam;
            float* left = osBlock.getChannelPointer(0);
            float* right = osBlock.getChannelPointer(1);
            
            for (int sample = 0; sample < osBlock.getNumSamples(); ++sample)
            {
                float mid = (left[sample] + right[sample]) * 0.5f;
                float side = (left[sample] - right[sample]) * 0.5f * width;
                left[sample] = mid + side;
                right[sample] = mid - side;
            }
        }
        
        oversampling->processSamplesDown(block);
    }
    else
    {
        // Similar processing without oversampling
        // [Processing code for non-oversampled path]
    }
    
    // Apply output gain
    buffer.applyGain(*outputParam);
}

float DistortionAudioProcessor::processDistortion(float input, DistortionType type, float drive)
{
    float driven = input * drive;
    
    switch (type)
    {
        case SoftClip:
            return std::tanh(driven);
            
        case HardClip:
            return juce::jlimit(-1.0f, 1.0f, driven);
            
        case Foldback:
        {
            float threshold = 0.8f;
            while (std::abs(driven) > threshold)
            {
                driven = fabs(fabs(fmod(driven - threshold, threshold * 4)) - threshold * 2) - threshold;
            }
            return driven;
        }
            
        case Waveshaper:
            return driven - (driven * driven * driven) / 3.0f;
            
        case Bitcrusher:
        {
            float bits = juce::jmap(drive, 1.0f, 25.0f, 16.0f, 4.0f);
            float steps = std::pow(2.0f, bits);
            return std::round(driven * steps) / steps;
        }
            
        default:
            return driven;
    }
}
