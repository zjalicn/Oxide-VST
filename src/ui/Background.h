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
        else
        {
            // Debug text to show if image loading failed
            g.setColour(juce::Colours::red);
            g.setFont(14.0f);
            g.drawText("Background image failed to load", getLocalBounds(), juce::Justification::centred);
        }
    }

private:
    juce::Image backgroundImage;

    void loadBackgroundImage()
    {
        juce::Logger::writeToLog("Attempting to load background image...");
        
        // 1. Try the binary resource
        int size = 0;
        const char* data = BinaryData::getNamedResource("bg_png", size);
        
        if (data != nullptr && size > 0)
        {
            juce::Logger::writeToLog("Found binary resource bg_png with size: " + juce::String(size));
            backgroundImage = juce::ImageCache::getFromMemory(data, size);
            
            if (backgroundImage.isValid())
            {
                juce::Logger::writeToLog("Successfully loaded background from binary resource");
                return;
            }
            else
            {
                juce::Logger::writeToLog("Binary resource found but image failed to load");
            }
        }
        else
        {
            juce::Logger::writeToLog("Binary resource bg_png not found");
        }
        
        // 2. Try direct file loading
        juce::File sourceDir = juce::File::getCurrentWorkingDirectory().getChildFile("src/resources");
        juce::Logger::writeToLog("Looking for image in: " + sourceDir.getFullPathName());
        
        juce::File pngFile = sourceDir.getChildFile("bg.png");
        juce::Logger::writeToLog("Full path to PNG: " + pngFile.getFullPathName());
        
        if (pngFile.existsAsFile())
        {
            juce::Logger::writeToLog("PNG file exists with size: " + juce::String(pngFile.getSize()));
            backgroundImage = juce::ImageCache::getFromFile(pngFile);
            
            if (backgroundImage.isValid())
            {
                juce::Logger::writeToLog("Successfully loaded background from file system");
            }
            else
            {
                juce::Logger::writeToLog("File exists but image failed to load - possibly corrupt or invalid format");
            }
        }
        else
        {
            juce::Logger::writeToLog("PNG file not found at path: " + pngFile.getFullPathName());
            
            // 3. Try some alternative paths
            juce::Array<juce::File> possiblePaths;
            possiblePaths.add(juce::File::getCurrentWorkingDirectory().getChildFile("build/src/resources/bg.png"));
            possiblePaths.add(juce::File::getSpecialLocation(juce::File::currentApplicationFile).getParentDirectory().getChildFile("resources/bg.png"));
            possiblePaths.add(juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory().getChildFile("resources/bg.png"));
            
            for (const auto& path : possiblePaths)
            {
                juce::Logger::writeToLog("Trying alternate path: " + path.getFullPathName());
                if (path.existsAsFile())
                {
                    juce::Logger::writeToLog("Found PNG at alternate path");
                    backgroundImage = juce::ImageCache::getFromFile(path);
                    
                    if (backgroundImage.isValid())
                    {
                        juce::Logger::writeToLog("Successfully loaded from alternate path");
                        return;
                    }
                }
            }
        }
        
        // If we got here, we failed to load the image
        juce::Logger::writeToLog("All attempts to load background image failed");
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Background)
};