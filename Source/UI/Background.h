#pragma once

#include <JuceHeader.h>

class Background : public juce::Component
{
public:
    Background()
    {
        setOpaque(true);

        // Try to load the background image from binary resources
        loadBackgroundImage();
    }

    void paint(juce::Graphics& g) override
    {
        // Fill with a gradient as a fallback
        juce::ColourGradient gradient(juce::Colour(0xff222222), 0.0f, 0.0f, 
                                     juce::Colour(0xff111111), 0.0f, (float)getHeight(), false);
        g.setGradientFill(gradient);
        g.fillAll();
        
        // Draw the background image if valid
        if (backgroundImage.isValid())
        {
            g.drawImageWithin(backgroundImage, 0, 0, getWidth(), getHeight(), 
                              juce::RectanglePlacement::stretchToFit);
        }
    }

private:
    juce::Image backgroundImage;

    void loadBackgroundImage()
    {
        // First, try to load using the filename we expect
        const char* bgData = nullptr;
        int bgSize = 0;
        
        // Check if the background resource exists with any of these names
        const char* possibleNames[] = {
            "bg_jpg", "bg", "background", "bg_png", "background_jpg", "background_png"
        };
        
        for (const auto& name : possibleNames)
        {
            if (BinaryData::getNamedResourceOriginalFilename(name) != nullptr)
            {
                bgData = BinaryData::getNamedResource(name, bgSize);
                if (bgData != nullptr && bgSize > 0)
                    break;
            }
        }

        // Directly try using Resources bg.jpg
        if (bgData == nullptr)
        {
            try {
                File resourceDir = File::getCurrentWorkingDirectory().getChildFile("Resources");
                File bgFile = resourceDir.getChildFile("bg.jpg");
                
                if (bgFile.existsAsFile())
                {
                    backgroundImage = juce::ImageFileFormat::loadFrom(bgFile);
                    return;
                }
            }
            catch (...) {
                // Silent catch if file loading fails
            }
        }
        
        // If we found binary data, load it
        if (bgData != nullptr && bgSize > 0)
        {
            backgroundImage = juce::ImageFileFormat::loadFrom(bgData, bgSize);
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Background)
};