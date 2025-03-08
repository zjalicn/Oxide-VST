#pragma once

#include <JuceHeader.h>

class Background : public juce::Component
{
public:
    Background()
    {
        setOpaque(true);
        loadBackgroundImage();
    }

    void paint(juce::Graphics& g) override
    {
        // Fallback dark background in case image doesn't load
        g.fillAll(juce::Colour(0xff1e1e1e));
        
        // Draw the background image if it loaded successfully
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
        // Get the current executable directory
        juce::File exeFile = juce::File::getSpecialLocation(juce::File::currentApplicationFile);
        juce::File exeDir = exeFile.getParentDirectory();
        
        // Check several potential locations for the Resources folder
        juce::Array<juce::File> possibleResourceDirs;
        
        // 1. Resources in the same directory as the executable
        possibleResourceDirs.add(exeDir.getChildFile("Resources"));
        
        // 2. Resources in the parent directory
        possibleResourceDirs.add(exeDir.getParentDirectory().getChildFile("Resources"));
        
        // 3. Current working directory
        possibleResourceDirs.add(juce::File::getCurrentWorkingDirectory().getChildFile("Resources"));
        
        // 4. One level up from current directory
        possibleResourceDirs.add(juce::File::getCurrentWorkingDirectory().getParentDirectory().getChildFile("Resources"));
        
        // Try each location
        for (const auto& resourceDir : possibleResourceDirs)
        {
            if (resourceDir.isDirectory())
            {
                juce::File bgFile = resourceDir.getChildFile("bg.jpg");
                if (bgFile.existsAsFile())
                {
                    backgroundImage = juce::ImageFileFormat::loadFrom(bgFile);
                    if (backgroundImage.isValid())
                        return;
                }
            }
        }
        
        // If we get here, we couldn't find the image file
        // Check if it's in BinaryData
        int size = 0;
        if (auto* data = BinaryData::getNamedResource("bg_jpg", size))
        {
            backgroundImage = juce::ImageFileFormat::loadFrom(data, size);
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Background)
};