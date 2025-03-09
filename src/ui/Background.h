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

    void paint(juce::Graphics &g) override
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
        // 1. Try the binary resource
        int size = 0;
        const char *data = BinaryData::getNamedResource("bg_png", size);

        if (data != nullptr && size > 0)
        {
            backgroundImage = juce::ImageCache::getFromMemory(data, size);

            if (backgroundImage.isValid())
            {
                return;
            }
        }

        // 2. Try direct file loading
        juce::File sourceDir = juce::File::getCurrentWorkingDirectory().getChildFile("src/resources");
        juce::File pngFile = sourceDir.getChildFile("bg.png");

        if (pngFile.existsAsFile())
        {
            backgroundImage = juce::ImageCache::getFromFile(pngFile);

            if (backgroundImage.isValid())
            {
                return;
            }
        }
        else
        {
            // 3. Try some alternative paths
            juce::Array<juce::File> possiblePaths;
            possiblePaths.add(juce::File::getCurrentWorkingDirectory().getChildFile("build/src/resources/bg.png"));
            possiblePaths.add(juce::File::getSpecialLocation(juce::File::currentApplicationFile).getParentDirectory().getChildFile("resources/bg.png"));
            possiblePaths.add(juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory().getChildFile("resources/bg.png"));

            for (const auto &path : possiblePaths)
            {
                if (path.existsAsFile())
                {
                    backgroundImage = juce::ImageCache::getFromFile(path);

                    if (backgroundImage.isValid())
                    {
                        return;
                    }
                }
            }
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Background)
};