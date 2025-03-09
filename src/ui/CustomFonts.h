#pragma once

#include <JuceHeader.h>
#include "BinaryData.h"

class CustomFonts
{
public:
    static const juce::Font getOldEnglishFont()
    {
        static auto typeface = juce::Typeface::createSystemTypefaceFor(
            BinaryData::old_english_hearts_ttf,
            BinaryData::old_english_hearts_ttfSize);
        return juce::Font(typeface);
    }
};