#pragma once

#include <JuceHeader.h>

class Background : public juce::Component
{
public:
    Background()
    {
        setOpaque(true);
        
        // Load the background image from binary resources
        juce::PNGImageFormat pngFormat;
        juce::JPEGImageFormat jpegFormat;
        
        int bgJpgSize = 0;
        const char* bgJpgData = nullptr;
        
        // Get resource data by name
        for (int i = 0; i < BinaryData::namedResourceListSize; ++i)
        {
            juce::String name = BinaryData::namedResourceList[i];
            
            if (name.containsIgnoreCase("bg_jpg") || name.containsIgnoreCase("bg.jpg") || 
                name.containsIgnoreCase("bgJpg") || name.containsIgnoreCase("background"))
            {
                bgJpgData = BinaryData::getNamedResource(name.toRawUTF8(), bgJpgSize);
                break;
            }
        }
        
        if (bgJpgData == nullptr || bgJpgSize == 0)
        {
            bgJpgData = BinaryData::bg_jpg;
            bgJpgSize = BinaryData::bg_jpgSize;
        }
        
        // Try loading with specific format readers first
        juce::MemoryInputStream stream(bgJpgData, bgJpgSize, false);
        backgroundImage = jpegFormat.decodeImage(stream);
        
        // If that didn't work, try the generic method
        if (!backgroundImage.isValid() && bgJpgData != nullptr && bgJpgSize > 0)
        {
            backgroundImage = juce::ImageFileFormat::loadFrom(bgJpgData, bgJpgSize);
        }
    }

    void paint(juce::Graphics& g) override
    {
        // Fill with black as a base
        g.fillAll(juce::Colours::black);
        
        // Draw the background image if valid
        if (backgroundImage.isValid())
        {
            g.drawImageWithin(backgroundImage, 0, 0, getWidth(), getHeight(), 
                              juce::RectanglePlacement::stretchToFit);
        }
    }

private:
    juce::Image backgroundImage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Background)
};