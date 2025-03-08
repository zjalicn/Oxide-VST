#include "MeterView.h"
#include "BinaryData.h"

class MeterWebBrowser : public juce::WebBrowserComponent
{
public:
    MeterWebBrowser(MeterView& owner) : ownerView(owner) {}
    
    bool pageAboutToLoad(const juce::String& url) override
    {
        // If URL starts with "oxide:", it's a control message
        if (url.startsWith("oxide:"))
        {
            // Parse the URL to get parameters
            juce::String params = url.fromFirstOccurrenceOf("oxide:", false, true);
            
            if (params.startsWith("inputGain="))
            {
                float value = params.fromFirstOccurrenceOf("inputGain=", false, true).getFloatValue();
                ownerView.notifyInputGainChanged(value);
                return false;
            }
            else if (params.startsWith("outputGain="))
            {
                float value = params.fromFirstOccurrenceOf("outputGain=", false, true).getFloatValue();
                ownerView.notifyOutputGainChanged(value);
                return false;
            }
            
            // Don't actually navigate to the URL
            return false;
        }
        
        // Allow normal navigation
        return true;
    }
    
private:
    MeterView& ownerView;
};

MeterView::MeterView()
    : inputGain(0.0f), outputGain(0.0f), 
      lastLeftLevel(0.0f), lastRightLevel(0.0f),
      outLeftLevel(0.0f), outRightLevel(0.0f),
      pageLoaded(false)
{
    webView.reset(new MeterWebBrowser(*this));
    webView->setWantsKeyboardFocus(false);
    webView->setFocusContainer(false);
    addAndMakeVisible(webView.get());
    
    juce::String htmlContent = juce::String(BinaryData::meter_html, BinaryData::meter_htmlSize);
    webView->goToURL("data:text/html;charset=utf-8," + htmlContent);
    
    // Start a timer to check if page is loaded
    startTimer(100);
}

MeterView::~MeterView()
{
    stopTimer();
}

void MeterView::timerCallback()
{
    // After a short delay, assume page is loaded and set flag
    stopTimer();
    pageLoaded = true;
    
    // Force an initial update to make sure meters show correctly
    updateLevels(5.0f, 5.0f); // Send a test signal
    updateLevels(lastLeftLevel, lastRightLevel);
}

void MeterView::paint(juce::Graphics& g)
{
}

void MeterView::resized()
{
    webView->setBounds(getLocalBounds());
}

void MeterView::updateLevels(float leftLevel, float rightLevel)
{
    if (!pageLoaded)
        return;
        
    // Store the last known levels
    lastLeftLevel = leftLevel;
    lastRightLevel = rightLevel;
    
    // Make sure we have reasonable values to display
    // Apply a minimum threshold to make faint signals visible
    leftLevel = std::max(leftLevel, 0.1f);
    rightLevel = std::max(rightLevel, 0.1f);
    
    // Calculate output levels based on input levels and output gain
    float gainMultiplier = std::pow(10.0f, outputGain / 20.0f);
    outLeftLevel = leftLevel * gainMultiplier;
    outRightLevel = rightLevel * gainMultiplier;
    
    // Ensure minimums for output too
    outLeftLevel = std::max(outLeftLevel, 0.1f);
    outRightLevel = std::max(outRightLevel, 0.1f);
    
    // Clamp output levels to 0-100%
    outLeftLevel = juce::jlimit(0.0f, 100.0f, outLeftLevel);
    outRightLevel = juce::jlimit(0.0f, 100.0f, outRightLevel);
    
    // Send all levels and gain values to the web view
    juce::String script = "setAudioState(" + 
                         juce::String(leftLevel) + ", " + 
                         juce::String(rightLevel) + ", " + 
                         juce::String(outLeftLevel) + ", " + 
                         juce::String(outRightLevel) + ", " + 
                         juce::String(inputGain) + ", " + 
                         juce::String(outputGain) + ")";
    
    webView->evaluateJavascript(script);
}

void MeterView::setInputGain(float newGain)
{
    inputGain = newGain;
    // Force an update of the UI with current levels but new gain settings
    updateLevels(lastLeftLevel, lastRightLevel);
}

void MeterView::setOutputGain(float newGain)
{
    outputGain = newGain;
    // Force an update of the UI with current levels but new gain settings
    updateLevels(lastLeftLevel, lastRightLevel);
}

void MeterView::notifyInputGainChanged(float newGain)
{
    inputGain = newGain;
    if (onInputGainChanged)
        onInputGainChanged(newGain);
    
    // Update display with new gain value
    updateLevels(lastLeftLevel, lastRightLevel);
}

void MeterView::notifyOutputGainChanged(float newGain)
{
    outputGain = newGain;
    if (onOutputGainChanged)
        onOutputGainChanged(newGain);
    
    // Update display with new gain value
    updateLevels(lastLeftLevel, lastRightLevel);
}